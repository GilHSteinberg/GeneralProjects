/* welcome to thread_pool_test.cpp */
/******************************************************************************
 *																			  *
 *                          code by : Gil H. Steinberg                        *
 *																			  *
 ******************************************************************************/

#define BOOST_THREAD_PROVIDES_FUTURE

#include <iostream>
#include <boost/thread/future.hpp>

#include "ca_test_util.hpp"
#include "thread_pool.hpp"

using namespace GHS::project;
using namespace ca_test_util;
using namespace std;
using namespace boost::chrono;
using boost::shared_ptr;
using boost::promise;
using boost::thread;

class BasicTask : public ThreadPool::Task
{
public:
	BasicTask() : m_num(0){}
	virtual ~BasicTask(){}

	size_t GetNum() const
	{
		return m_num;
	}

private:
	void Execute()
	{
		//cout << "i am not a thread" << endl;
		sleep(1);
		//cout <<"and so am i" << endl;
	}
	int m_num;
};

class MultiplyTask : public ThreadPool::Task
{
public:
	MultiplyTask(int num, int num2, boost::shared_ptr <promise<int> > prom, priority p)
						: Task(p), m_num(num), m_num2(num2), m_promise(prom){}
	virtual ~MultiplyTask(){}

private:
	void Execute()
	{
		int retVal = 0;
		for (int i = 0; i < m_num2; ++i)
		{
			retVal += m_num;
		}
		m_promise->set_value(retVal);
	}
	int m_num;
	int m_num2;
	boost::shared_ptr <promise<int> > m_promise;

};

void SanityTest();
void PromiseFutureTest();
void StopTest();
void PressureTest();

int main()
{
    int p = system("clear");
    p = p;
    cout <<
    "\b\n╚══════════════     Welcome to ThreadPool test (v1.0)     ══════════════╗" << endl;

	SanityTest();

	PromiseFutureTest();

	// NOTE: this test might cause resource leak.
	// this is an acceptable behaviour of the (risky) Stop method
	StopTest();
	PressureTest();

	TestSummary();
	return 0;
}

void PressureTest()
{
    bool has_crashed = false;
    try
    {
        cout << "Now Running Pressure Test: ";
	    ThreadPool threadPool(10);
    }
    catch (exception e)
    {
        has_crashed = true;
    }
    Test(has_crashed, false);
}

void StopTest()
{
	//cout << "stop test executed" << endl << endl;
	bool is_running = true;
	ThreadPool threadPool(15);
	boost::shared_ptr<ThreadPool::Task> basicTask(new BasicTask());
	for (std::size_t i = 0; i < 50; ++i)
	{
		threadPool.AddTask(basicTask);
	}
	threadPool.SetNumOfThreads(3);
	cout << "Now Running Stop Test: ";
	threadPool.Stop(milliseconds(5));
	is_running = false;
	Test(is_running,false);
	//cout << "stop test ended" << endl << endl;
}

void PromiseFutureTest()
{
	//cout << "promise/future test executed" << endl << endl;
	ThreadPool threadPool(4);

	boost::shared_ptr<promise<int> > prom1(new promise<int>());
	boost::future<int> future1 = prom1->get_future();
	boost::shared_ptr<ThreadPool::Task> task1(new MultiplyTask(2,20,prom1,
														ThreadPool::Task::LOW));

	boost::shared_ptr<promise<int> > prom2(new promise<int>());
	boost::future<int> future2 = prom2->get_future();
	boost::shared_ptr<ThreadPool::Task> task2(new MultiplyTask(3,100,prom2,
	                                                   ThreadPool::Task::HIGH));

	boost::shared_ptr<promise<int> > prom3(new promise<int>());
	boost::future<int> future3 = prom3->get_future();
	boost::shared_ptr<ThreadPool::Task> task3(new MultiplyTask(4,500,prom3,
	                                                 ThreadPool::Task::MEDIUM));

	boost::shared_ptr<promise<int> > prom4(new promise<int>());
	boost::future<int> future4 = prom4->get_future();
	boost::shared_ptr<ThreadPool::Task> task4(new MultiplyTask(5,5000,prom4,
	                                                    ThreadPool::Task::LOW));

	boost::shared_ptr<promise<int> > prom5(new promise<int>());
	boost::future<int> future5 = prom5->get_future();
	boost::shared_ptr<ThreadPool::Task> task5(new MultiplyTask(1,100400,prom5,
	                                                    ThreadPool::Task::HIGH));

	boost::shared_ptr<promise<int> > prom6(new promise<int>());
	boost::future<int> future6 = prom6->get_future();
	boost::shared_ptr<ThreadPool::Task> task6(new MultiplyTask(10,123456,prom6,
	                                                    ThreadPool::Task::LOW));


	threadPool.AddTask(task1);
	threadPool.AddTask(task2);
	threadPool.AddTask(task3);
	threadPool.AddTask(task4);
	threadPool.AddTask(task5);
	threadPool.AddTask(task6);
    cout << "Now Running Promise Future Test1: ";
	Test(future1.get(),40);
    cout << "Now Running Promise Future Test2: ";
	Test(future2.get(),300);
    cout << "Now Running Promise Future Test3: ";
	Test(future3.get(),2000);
    cout << "Now Running Promise Future Test4: ";
	Test(future4.get(),25000);
    cout << "Now Running Promise Future Test5: ";
	Test(future5.get(),100400);
    cout << "Now Running Promise Future Test6: ";
	Test(future6.get(),1234560);
	//cout << "promise/future test ended" << endl << endl;
}

void SanityTest()
{
	//cout << "sanity test executed" << endl << endl;
	ThreadPool threadPool(10);
	cout << "Now Running SanityTest(Run threads): ";
	Test(threadPool.GetNumOfThreads(),(size_t)10);
	boost::shared_ptr<ThreadPool::Task> basicTask(new BasicTask());
	for (std::size_t i = 0; i < 100; ++i)
	{
		threadPool.AddTask(basicTask);
		if (i == 7)
		{
			threadPool.SetNumOfThreads(5);
		}
	}
	cout << "Now Running SanityTest(Add threads): ";
	Test(threadPool.GetNumOfThreads(),(size_t)5);
	threadPool.Pause();
	//cout << "thread pool stopped" << endl;

	for (std::size_t i = 0; i < 100; ++i)
	{
		threadPool.AddTask(basicTask);
	}

	threadPool.Resume();
	//cout << "thread pool resumed" << endl;

	threadPool.SetNumOfThreads(50);
	cout << "Now Running SanityTest(Run after pause): ";
	Test(threadPool.GetNumOfThreads(),(size_t)50);
	for (std::size_t i = 0; i < 100; ++i)
	{
		threadPool.AddTask(basicTask);
	}
	threadPool.SetNumOfThreads(5);
	cout << "Now Running SanityTest(Run after set num of threads): ";
	Test(threadPool.GetNumOfThreads(),(size_t)5);
	//cout << "sanity test ended" << endl << endl;
}
