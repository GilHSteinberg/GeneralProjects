/******************************************************************************
 * 																			  *
 *							CREATED BY: Gil						              *
 *							CREATED ON: 27-08-2020  			 			  *
 *							REVIEWER: 	 					                  *
 * 																		      *
 ******************************************************************************/

#ifndef GHS_WAITABLEQUEUE_HPP
#define GHS_WAITABLEQUEUE_HPP

#include <queue>                       // queue
#include <boost/noncopyable.hpp>       // noncopyable
#include <boost/chrono.hpp>            // nanoseconds
#include <boost/thread/mutex.hpp>      // boost::mutex
#include <boost/thread/condition.hpp>  // boost::condition


namespace GHS
{
namespace project
{
//╔═════════════════════════      WaitableQueue     ═══════════════════════════╗
template <class T, class Container = std::queue<T> >
class WaitableQueue : private boost::noncopyable
{
public:
    explicit WaitableQueue() = default;
    ~WaitableQueue() = default;

    void Push(const T& data);

    void Pop(T &out);
    bool Pop(T &out, boost::chrono::nanoseconds timeout);

    bool IsEmpty() const;

private:
    Container m_container;
    boost::mutex m_mutex;
    boost::condition_variable m_pushSignal;
};

//╚═════════════════════════      WaitableQueue     ═══════════════════════════╝

//╔═════════════════════════      PriorityQueue     ═══════════════════════════╗
template <typename T>
class PriorityQueue : private boost::noncopyable
{
public:
    explicit PriorityQueue() = default;
    ~PriorityQueue() = default;

    const T &front();
    void pop();
    void push( const T& value);
    bool empty() const;

private:
    std::priority_queue<T> m_queue;
};

template<typename T>
const T &PriorityQueue<T>::front()
{
    return m_queue.top();
}

template<typename T>
void PriorityQueue<T>::pop()
{
    m_queue.pop();
}

template<typename T>
void PriorityQueue<T>::push(const T &value)
{
    m_queue.push(value);
}

template<typename T>
bool PriorityQueue<T>::empty() const
{
    return m_queue.empty();
}

//╚═════════════════════════      PriorityQueue     ═══════════════════════════╝

//╔═════════════════════════          utils         ═══════════════════════════╗
static inline boost::chrono::system_clock::time_point
                GetTimePoint(const boost::chrono::nanoseconds &nano)
{
    boost::chrono::system_clock::time_point wakeUpTime
                    = boost::chrono::system_clock::now() + nano;
    return wakeUpTime;
}
//╚═════════════════════════          utils         ═══════════════════════════╝

//╔═════════════════════════      WaitableQueue     ═══════════════════════════╗
template<class T, class Container>
void WaitableQueue<T, Container>::Push(const T &data)
{
    boost::unique_lock<boost::mutex> lock(m_mutex);
    m_container.push(data);
    m_pushSignal.notify_one();
}

template<class T, class Container>
void WaitableQueue<T, Container>::Pop(T &out)
{
    boost::unique_lock<boost::mutex> lock(m_mutex);

    while (IsEmpty())
    {
        m_pushSignal.wait(lock);
    }

    out = m_container.front();
    m_container.pop();
}

template<class T, class Container>
bool WaitableQueue<T, Container>::Pop(T &out,
                                      boost::chrono::nanoseconds timeout)
{
    boost::chrono::system_clock::time_point topTime = GetTimePoint(timeout);
    boost::unique_lock<boost::mutex> lock(m_mutex);

    while (IsEmpty())
    {
        if (boost::cv_status::timeout ==
                                    m_pushSignal.wait_until(lock, topTime))
        {
            return false;
        }
    }

    out = m_container.front();
    m_container.pop();
    return true;
}

template<class T, class Container>
bool WaitableQueue<T, Container>::IsEmpty() const
{
    return m_container.empty();
}
//╚═════════════════════════      WaitableQueue     ═══════════════════════════╝

}//namespace project
}//namespace GHS
#endif // GHS_WAITABLEQUEUE_HPP
