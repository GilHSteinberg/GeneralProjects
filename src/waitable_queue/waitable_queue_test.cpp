/******************************************************************************
 * 																			  *
 *							CREATED BY: Gil						              *
 *							CREATED ON: 27-08-2020  			 			  *
 *							REVIEWER: 	 					                  *
 * 																		      *
 ******************************************************************************/

#include <boost/thread.hpp>     // boost::thread
#include <cstdio>

#include "waitable_queue.hpp"

using namespace ilrd;
using namespace project;

const static int S_NUM = 5000;

void TryPop(WaitableQueue<int> *wq);
void TryPopTimeout(WaitableQueue<int> *wq, bool *result);
void TryPush(WaitableQueue<int> *wq);

size_t g_numOfChecks = 0;
const int g_numOfTests = 6;
// array of function pointers
bool (*g_testFunc[g_numOfTests])() = {0};
// array of function names as string
std::string g_testNames[g_numOfTests];

static void _SetUpTables();
bool CtorDtorTest();
bool PushTest();
bool PopBeforePushTest();
bool PopAfterPushTest();
bool PopWithTimeoutWithPushTest();
bool PopWithTimeoutWithoutPushTest();

int main()
{
    int randNum = (rand() % 23) + g_numOfTests;

    _SetUpTables();

    for (int i = 0; randNum > i; ++i, ++g_numOfChecks)
    {
        RunTest(g_testNames[i % g_numOfTests].c_str(),
                               g_testFunc[i % g_numOfTests], 1);
    }

    for (int i = 0; g_numOfTests > i; ++i, ++g_numOfChecks)
    {
        RunTest(g_testNames[i].c_str(),
                               g_testFunc[i], 1);
    }

    char buffer[BUFSIZ] = {0};
    sprintf(buffer, "number of tests success: %ld\n", g_numOfChecks);
    PrintYellow(buffer);

    return 0;
}

static void _SetUpTables()
{
    g_testFunc[0]=&CtorDtorTest;
    g_testNames[0]="CtorDtorTest";
    g_testFunc[1]=&PushTest;
    g_testNames[1]="PushTest";
    g_testFunc[2]=&PopAfterPushTest;
    g_testNames[2]="PopAfterPushTest";
    g_testFunc[3]=&PopBeforePushTest;
    g_testNames[3]="PopBeforePushTest";
    g_testFunc[4]=&PopWithTimeoutWithPushTest;
    g_testNames[4]="PopWithTimeoutWithPushTest";
    g_testFunc[5]=&PopWithTimeoutWithoutPushTest;
    g_testNames[5]="PopWithTimeoutWithoutPushTest";
}

void TryPop(WaitableQueue<int> *wq)
{
    int test = 0;
    wq->Pop(test);
}

void TryPopTimeout(WaitableQueue<int> *wq, bool *result)
{
    int test = 0;
    boost::chrono::nanoseconds nano(1000);
    *result = wq->Pop(test,nano);
}


void TryPush(WaitableQueue<int> *wq)
{
    wq->Push((rand() % 23) + g_numOfTests);
}

bool CtorDtorTest()
{
    WaitableQueue<int> wq;
    return (wq.IsEmpty());
}

bool PushTest()
{
    WaitableQueue<int> wq;

    boost::thread threads[10];

    for (int i = 0; 10 > i; ++i)
    {
        threads[i] = boost::thread(TryPush, &wq);
    }

    for (int i = 0; 10 > i; ++i)
    {
        threads[i].join();
    }

    return (!wq.IsEmpty());
}

bool PopBeforePushTest()
{
    WaitableQueue<int> wq;

    boost::thread popThreads[10];
    boost::thread pushThreads[10];

    for (int i = 0; 10 > i; ++i)
    {
        popThreads[i] = boost::thread(TryPop, &wq);
    }

    for (int i = 0; 10 > i; ++i)
    {
        pushThreads[i] = boost::thread(TryPush, &wq);
    }

    for (int i = 0; 10 > i; ++i)
    {
        popThreads[i].join();
    }

    return wq.IsEmpty();
}

bool PopAfterPushTest()
{
    WaitableQueue<int> wq;

    boost::thread popThreads[10];
    boost::thread pushThreads[10];

    for (int i = 0; 10 > i; ++i)
    {
        pushThreads[i] = boost::thread(TryPush, &wq);
    }

    for (int i = 0; 10 > i; ++i)
    {
        popThreads[i] = boost::thread(TryPop, &wq);
    }

    for (int i = 0; 10 > i; ++i)
    {
        popThreads[i].join();
    }

    return wq.IsEmpty();
}

bool PopWithTimeoutWithPushTest()
{
    WaitableQueue<int> wq;

    boost::thread popThreads[10];
    boost::thread pushThreads[10];

    for (int i = 0; 10 > i; ++i)
    {
        pushThreads[i] = boost::thread(TryPush, &wq);
    }

    for (int i = 0; 10 > i; ++i)
    {
        pushThreads[i].join();
    }

    bool result[10] ={false};
    for (int i = 0; 10 > i; ++i)
    {
        popThreads[i] = boost::thread(TryPopTimeout, &wq, result + i);
    }

    for (int i = 0; 10 > i; ++i)
    {
        popThreads[i].join();
    }

    for (int i = 0; 10 > i; ++i)
    {
        if (!result[i])
        {
            return false;
        }
    }

    return wq.IsEmpty();
}

bool PopWithTimeoutWithoutPushTest()
{
    WaitableQueue<int> wq;

    boost::thread popThreads[10];

    bool result[10] ={false};
    for (int i = 0; 10 > i; ++i)
    {
        popThreads[i] = boost::thread(TryPopTimeout, &wq, result + i);
    }

    for (int i = 0; 10 > i; ++i)
    {
        popThreads[i].join();
    }

    for (int i = 0; 10 > i; ++i)
    {
        if (result[i])
        {
            return false;
        }
    }

    return wq.IsEmpty();
}
