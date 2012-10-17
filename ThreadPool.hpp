#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include "ConcurrentQueue.hpp"

namespace oneandone  {
namespace nms {

using boost::shared_ptr;

template <typename Task>
class ThreadPool
{
public:
    ThreadPool(boost::function<void ()> on_worker_thread_start = NULL,
               boost::function<void ()> on_worker_thread_stop = NULL)
        : on_worker_thread_start_(on_worker_thread_start),
          on_worker_thread_stop_(on_worker_thread_stop)
    {
    }

public:
    void Start(size_t min_threads, size_t max_threads)
    {
        min_threads_ = min_threads;
        max_threads_ = max_threads;
        busy_threads_= 0;
        queue_.Start();
        boost::mutex::scoped_lock lock(mutex_);
        {
            total_threads_ = min_threads;
            for(size_t i = 0; i < total_threads_; ++i)
                boost::thread detached_thread(boost::bind(&ThreadPool::RunTask, this));
        }
    }

    void AddTask(const Task& task)
    {
        {
            // If all the worker threads in the pool are busy, create a new one.
            boost::mutex::scoped_lock lock(mutex_);
            if (busy_threads_ == total_threads_ && total_threads_ < max_threads_) {
                boost::thread detached_thread(boost::bind(&ThreadPool::RunTask, this));
                ++total_threads_;
            }
        }
        queue_.Push(task);
    }

    void Shutdown(bool wait = true)
    {
        if (wait)
            queue_.Wait();
        queue_.Stop();
        boost::unique_lock<boost::mutex> lock(mutex_);
        while (total_threads_ > 0)
            threads_terminated_.wait(lock);
    }

private:
    void RunTask()
    {
        if (on_worker_thread_start_)
            on_worker_thread_start_();
        while(true) {
            try {
                Task task;
                queue_.Pop(task);
                {
                    boost::mutex::scoped_lock lock(mutex_);
                    ++busy_threads_;
                }
                task->Run();
                {
                    boost::mutex::scoped_lock lock(mutex_);
                    --busy_threads_;
                    // If only less than half of the threads are busy, terminate this one
                    if (busy_threads_ < total_threads_/2 && total_threads_ > min_threads_)
                        break;
                }
            } catch (const ConcurrentQueueInterruptException& e) {
                break;
            }
        }
        if (on_worker_thread_stop_)
            on_worker_thread_stop_();

        {
            boost::mutex::scoped_lock lock(mutex_);
            --total_threads_;
        }
        threads_terminated_.notify_one();
    }

private:
    ConcurrentQueue<Task>       queue_;
    boost::function<void ()>    on_worker_thread_start_;
    boost::function<void ()>    on_worker_thread_stop_;
    size_t                      min_threads_;
    size_t                      max_threads_;
    size_t                      busy_threads_;
    size_t                      total_threads_;
    boost::condition_variable   threads_terminated_;
    boost::mutex                mutex_;
};

}   // namespace oneandone
}   // namespace nms

#endif // THREADPOOL_HPP
