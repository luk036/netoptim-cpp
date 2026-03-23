/*!
 * @file ThreadPool.h
 * @brief Thread pool implementation for parallel task execution
 *
 * This module provides a simple thread pool implementation that allows
 * for parallel execution of tasks. Tasks are submitted to the pool and
 * executed by worker threads.
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#define _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

/*!
 * @brief Thread pool for parallel task execution
 *
 * This class implements a thread pool that manages a fixed number of
 * worker threads. Tasks can be submitted to the pool and are executed
 * asynchronously by available worker threads.
 *
 * The thread pool uses a work-stealing queue pattern where worker threads
 * wait for tasks and execute them as they become available. The pool
 * automatically manages thread lifecycle and task queue synchronization.
 */
class ThreadPool {
  public:
    /*!
     * @brief Construct a new thread pool
     *
     * Creates a thread pool with the specified number of worker threads.
     * Each worker thread is immediately started and waits for tasks to execute.
     *
     * @param[in] threads Number of worker threads to create
     */
    ThreadPool(size_t);

    /*!
     * @brief Enqueue a task for execution
     *
     * This method adds a task to the thread pool's queue. The task will be
     * executed by an available worker thread. The method returns a future
     * that can be used to retrieve the result or wait for completion.
     *
     * @tparam F Type of the callable object
     * @tparam Args Types of the arguments
     * @param[in] f The callable object to execute
     * @param[in] args Arguments to pass to the callable
     * @return std::future<typename std::result_of<F(Args...)>::type> A future
     *         containing the result of the task
     */
    template <class F, class... Args> auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    /*!
     * @brief Destroy the thread pool
     *
     * The destructor signals all worker threads to stop, waits for them
     * to complete their current tasks, and then joins all threads.
     */
    ~ThreadPool();

  private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void()>> tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// the constructor just launches some amount of workers
/*!
 * @brief Construct a new thread pool
 *
 * Creates worker threads that wait for tasks to be enqueued. Each worker
 * runs in an infinite loop, waiting on a condition variable for tasks
 * to become available.
 *
 * @param[in] threads Number of worker threads to create
 */
inline ThreadPool::ThreadPool(size_t threads) : stop(false) {
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock,
                                         [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                task();
            }
        });
}

// add new work item to the pool
/*!
 * @brief Enqueue a task for asynchronous execution
 *
 * Wraps the callable and its arguments into a packaged_task and adds it
 * to the task queue. A future is returned that will contain the result
 * when the task completes.
 *
 * @tparam F Type of the callable object
 * @tparam Args Types of the arguments
 * @param[in] f The callable object to execute
 * @param[in] args Arguments to pass to the callable
 * @return std::future<typename std::result_of<F(Args...)>::type> Future containing the result
 * @throws std::runtime_error if enqueue is called after the pool has been stopped
 */
template <class F, class... Args> auto ThreadPool::enqueue(F &&f, Args &&...args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
/*!
 * @brief Destroy the thread pool and join all worker threads
 *
 * Signals all worker threads to stop by setting the stop flag and
 * waking all threads. Then waits for each thread to finish by calling
 * join() on each worker thread.
 */
inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) worker.join();
}

#endif
