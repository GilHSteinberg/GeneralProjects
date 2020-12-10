/* welcome to thread_pool.hpp */
/******************************************************************************
 *																			  *
 *                          code by : Gil H. Steinberg                        *
 *																			  *
 ******************************************************************************/

#ifndef GHS_THREAD_POOL_HPP
#define GHS_THREAD_POOL_HPP

#include <boost/noncopyable.hpp>// noncopyable
#include <boost/thread.hpp>     // threads
#include <boost/chrono.hpp>     // time_point, milliseconds
#include <boost/atomic.hpp>     // atomic variables

#include "waitable_queue.hpp"

#if __cplusplus<201103L
#define noexcept throw()
#endif

namespace GHS
{
namespace project
{

class ThreadPool: boost::noncopyable
{
public:
	explicit ThreadPool(size_t numOfThreads);
	~ThreadPool();

	class Task
	{
	public:
		enum priority
		{
			LOW,
			MEDIUM,
			HIGH,
			SUPREME
		};

		explicit Task(priority = MEDIUM);
		virtual ~Task();

		bool operator<(const Task &other) const noexcept;
	private:
		friend class ThreadPool;

		virtual void Execute() = 0;
		priority m_priority;
	};

	void AddTask(boost::shared_ptr<Task> newTask);
	void Stop(boost::chrono::milliseconds timeout);
	void Pause() noexcept;
	void Resume() noexcept;
	void SetNumOfThreads(size_t newNumOfThreads);
	size_t GetNumOfThreads() const;

private:
	boost::atomic<bool> m_threadsArePaused;

	WaitableQueue<boost::shared_ptr<Task>,
					      PriorityQueue<boost::shared_ptr<Task> > > m_TaskQueue;
	std::map<boost::thread::id,boost::shared_ptr<boost::thread> > m_ThreadGroup;

	mutable boost::mutex m_mapMutex;
	mutable boost::mutex m_conditionVariableMutex;

	boost::condition_variable m_conditionVariable;

	void InitAndRunThread();

	void KillAllThreads();

	void AddThreads(size_t threadAmountToAdd);
	void ReducePoolSize(size_t threadAmountToReduce);
	boost::shared_ptr<boost::thread> EraseThread(boost::thread::id id);
	void JoinAllThreads();
	void AddCloseThreadTask(boost::shared_ptr
	                              <boost::promise<boost::thread::id> > promise);

	class ThreadCloser : public Task
	{
	public:
		explicit ThreadCloser(boost::shared_ptr
		                            <boost::promise<boost::thread::id> > prom);
		virtual ~ThreadCloser() = default;;

	private:
		virtual void Execute();
		boost::shared_ptr<boost::promise<boost::thread::id> > m_threadToRemove;
	};
	class VoidTask : public Task
	{
	public:
		explicit VoidTask();
		virtual ~VoidTask() = default;

	private:
		virtual void Execute();
	};
};

} //namespace project
} //namespace GHS

#endif /* ifdef GHS_THREAD_POOL_HPP */

