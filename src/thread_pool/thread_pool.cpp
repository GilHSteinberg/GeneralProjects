/* welcome to thread_pool.cpp */
/******************************************************************************
 * 																			  *
 *							CREATED BY: Gil						              *
 * 																		      *
 ******************************************************************************/

#define BOOST_THREAD_PROVIDES_FUTURE //needed for boost::future to work

#include <stdexcept>                // exceptions

#include <boost/thread/future.hpp>  // future

#include "thread_pool.hpp"

using namespace boost::chrono;
using boost::chrono::milliseconds;
using boost::chrono::nanoseconds;
using namespace boost::this_thread;
using boost::this_thread::sleep_until;
using boost::this_thread::get_id;

using boost::mutex;
using boost::thread;
using boost::shared_ptr;
using boost::bind;
using std::map;
using boost::future;
using boost::promise;

//╔═══════════════════════   static utils and defs   ══════════════════════════╗
typedef map<thread::id, shared_ptr<thread> > thread_map;
class remove_me : public std::runtime_error
{
public:
	explicit remove_me() : std::runtime_error(""){}
};

struct ThreadJoiner
{
    void operator()
    (std::pair<boost::thread::id,boost::shared_ptr<boost::thread> > i)
    {
        i.second->join();
    }
};

static void MakeThreadCancelable()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
}

static void KillThread(shared_ptr<thread> thrPtr)
{
	pthread_t toKill = thrPtr->native_handle();

	pthread_cancel(toKill);
	pthread_join(toKill, NULL);
}
//╚═══════════════════════   static utils and defs   ══════════════════════════╝

namespace GHS
{
namespace project
{

//╔═════════════════════════    ThreadPool(API)     ═══════════════════════════╗
ThreadPool::ThreadPool(size_t numOfThreads): m_threadsArePaused(false)
{
    AddThreads(numOfThreads);
}

ThreadPool::~ThreadPool()
{
	size_t numOfThreads = GetNumOfThreads();

	for (size_t i = 0; i < numOfThreads; ++i)
	{
		shared_ptr<Task> empty(new VoidTask());
		m_TaskQueue.Push(empty);
	}
    JoinAllThreads();
}

void ThreadPool::AddTask(shared_ptr<Task> newTask)
{
	m_TaskQueue.Push(newTask);
}

void ThreadPool::Stop(milliseconds timeout)
{
	size_t numOfThreads = GetNumOfThreads();

	time_point<steady_clock> wakeupTime =
			(steady_clock::now()+ milliseconds(timeout));
	sleep_until(wakeupTime);

	KillAllThreads();
    AddThreads(numOfThreads);
}

void ThreadPool::Pause() noexcept
{
    m_threadsArePaused = true;
}

void ThreadPool::Resume() noexcept
{
    m_threadsArePaused = false;
	m_conditionVariable.notify_all();
}

void ThreadPool::SetNumOfThreads(size_t newNumOfThreads)
{
	size_t currentNumOfThreads = GetNumOfThreads();

	if (newNumOfThreads > currentNumOfThreads)
	{
        AddThreads(newNumOfThreads - currentNumOfThreads);
	}
	else if (newNumOfThreads < currentNumOfThreads)
	{
        ReducePoolSize(currentNumOfThreads - newNumOfThreads);
	}
}

size_t ThreadPool::GetNumOfThreads() const
{
	mutex::scoped_lock scopeLock(m_mapMutex);
	return (m_ThreadGroup.size());
}
//╚═════════════════════════    ThreadPool(API)     ═══════════════════════════╝

//╔═════════════════════════    ThreadPool(IMP)     ═══════════════════════════╗
void ThreadPool::InitAndRunThread()
{
	MakeThreadCancelable();
	shared_ptr<Task> current;
	bool ThreadIsAlive = true;
	while(ThreadIsAlive)
	{
		bool Execute = true;
		m_TaskQueue.Pop(current);
		while (m_threadsArePaused)
		{
			mutex::scoped_lock lock(m_conditionVariableMutex);
			m_conditionVariable.wait(lock); //waits for notify all
			m_TaskQueue.Push(current);
			Execute = false;
		}
		try
		{
			if (Execute)
			{
				current->Execute();
			}
		}
		catch (remove_me &except)
		{
			ThreadIsAlive = false;
		}
	}
}


void ThreadPool::KillAllThreads()
{
	mutex::scoped_lock scopeLock(m_mapMutex);
	thread_map::iterator next_it;
	for (thread_map::iterator it = m_ThreadGroup.begin(), next_it = it;
	     it != m_ThreadGroup.end();
	     it = next_it)
	{
		next_it++;
		KillThread(it->second);
		m_ThreadGroup.erase(it);
	}
}

void ThreadPool::AddThreads(size_t threadAmountToAdd)
{
	mutex::scoped_lock lock(m_mapMutex);
	for (size_t i = 0; i < threadAmountToAdd; ++i)
	{
		shared_ptr<thread> threadPtr(new thread
                             (bind(&ThreadPool::InitAndRunThread, this)));
		m_ThreadGroup[threadPtr->get_id()] = threadPtr;
	}
}

void ThreadPool::ReducePoolSize(size_t numToRemove)
{
	for (size_t i = 0; i < numToRemove; ++i)
	{
		shared_ptr<promise<thread::id> > prom(new promise<thread::id>());
		future<thread::id> threadFuture = prom->get_future();

		AddCloseThreadTask(prom);
		thread::id threadId = threadFuture.get();

		shared_ptr<thread> threadToJoin = EraseThread(threadId);
		threadToJoin->join();
	}
}

shared_ptr<thread> ThreadPool::EraseThread(thread::id id)
{
	mutex::scoped_lock lock(m_mapMutex);
	thread_map::iterator it = m_ThreadGroup.find(id);
	shared_ptr<thread> threadPtr(it->second);
	m_ThreadGroup.erase(it);
	return threadPtr;
}

void ThreadPool::JoinAllThreads()
{
    std::for_each(m_ThreadGroup.begin(), m_ThreadGroup.end(), ThreadJoiner());
}

void ThreadPool::AddCloseThreadTask(shared_ptr<promise<thread::id> > promise)
{
	shared_ptr<Task> closingTask(new ThreadCloser(promise));
	m_TaskQueue.Push(closingTask);
}

// ═════════════════════════    ThreadPool::Task     ═══════════════════════════
ThreadPool::Task::Task(ThreadPool::Task::priority priority): m_priority(priority)
{
	// empty
}

ThreadPool::Task::~Task()
{
	// empty
}

bool ThreadPool::Task::operator<(const ThreadPool::Task &other) const noexcept
{
	return (m_priority < other.m_priority);
}
// ═══════════════════    ThreadPool::ThreadCloser     ═════════════════════════
ThreadPool::ThreadCloser::ThreadCloser(shared_ptr<promise<thread::id> > prom)
										: Task(SUPREME), m_threadToRemove(prom)
{
	// empty
}

void ThreadPool::ThreadCloser::Execute()
{
	m_threadToRemove->set_value(get_id());
	throw remove_me();
}
// ═══════════════════    ThreadPool::VoidTask     ═════════════════════════════
ThreadPool::VoidTask::VoidTask() : Task(static_cast<priority>(LOW - 1))
{
	// empty
}


void ThreadPool::VoidTask::Execute()
{
	throw remove_me();
}
//╚═════════════════════════    ThreadPool(IMP)     ═══════════════════════════╝
} // namespace project
} // namespace GHS


