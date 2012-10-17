#ifndef CONCURRENTQUEUE_HPP
#define CONCURRENTQUEUE_HPP

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <deque>
#include <stdexcept>

namespace oneandone  {
namespace nms {

class ConcurrentQueueInterruptException : public std::runtime_error
{
public:
    explicit ConcurrentQueueInterruptException(const std::string &message)
        : runtime_error(message) {}
};


template <typename T>
class ConcurrentQueue
{
public:
    ConcurrentQueue()
    {
        Start();
    }

public:
    void Push(const T &value)
    {
        boost::mutex::scoped_lock lock(queue_mutex_);
        if (!stop_) {
            queue_.push_back(value);
            ready_.notify_one();
        }
    }

    void Pop(T &value)
    {
        boost::mutex::scoped_lock lock(queue_mutex_);
        while (queue_.empty() && !stop_) {
            ready_.wait(lock);
        }
        if (stop_) {
            throw ConcurrentQueueInterruptException("Queue interrupted");
        }
        value = queue_.front();
        queue_.pop_front();

        if (queue_.empty())
            empty_.notify_one();
    }

    void Start()
    {
        stop_ = false;
    }

    void Wait()
    {
        boost::mutex::scoped_lock lock(queue_mutex_);
        while (!queue_.empty()) {
            empty_.wait(lock);
        }
    }

    void Stop()
    {
        boost::mutex::scoped_lock lock(queue_mutex_);
        stop_ = true;
        ready_.notify_all();
        queue_.clear();
    }

private:
    std::deque<T>               queue_;
    boost::mutex                queue_mutex_;
    boost::condition_variable   ready_;
    boost::condition_variable   empty_;
    bool                        stop_;
};

}   // namespace oneandone
}   // namespace nms

#endif // CONCURRENTQUEUE_HPP
