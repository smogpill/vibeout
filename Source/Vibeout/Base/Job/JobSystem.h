// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Job.h"
#include "Vibeout/Base/Singleton.h"

class JobSystem : public Singleton<JobSystem>
{
public:
    JobSystem(uint numThreads = std::thread::hardware_concurrency());
    ~JobSystem();

    /// Enqueue a regular function/lambda as a job
    template<typename Callable>
    void Enqueue(Callable&& callable);

    /// Helper function to enqueue blocking jobs
    template<typename Callable>
    void EnqueueBlocking(Callable&& callable);

    /// Enqueue a coroutine and return a handle to await its result
    template<typename CoroutineJob>
    auto EnqueueCoroutine(CoroutineJob&& coroTask);

    /// Run a blocking function on a dedicated thread (for I/O)
    template<typename Callable>
    auto RunBlockingJob(Callable&& callable);

    /// For the resource manager's dependency waiting
    void ResumeCoroutine(std::coroutine_handle<> h) { Enqueue([h]() { h.resume(); }); }
    void Stop();
    bool IsRunning() const { return _running; }

private:
    void WorkerThread();

    std::vector<std::jthread> _threads;
    std::deque<Job> _jobQueue;
    std::mutex _queueMutex;
    std::condition_variable _condition;
    std::atomic<bool> _running = true;

    // Separate queue and thread for blocking operations (like file I/O)
    std::deque<Job> _blockingQueue;
    std::mutex _blockingMutex;
    std::condition_variable _blockingCondition;
    std::jthread _blockingThread;
};

template<typename Callable>
void JobSystem::Enqueue(Callable&& callable)
{
    {
        std::unique_lock lock(_queueMutex);
        _jobQueue.emplace_back(std::forward<Callable>(callable));
    }
    _condition.notify_one();
}

template<typename Callable>
void JobSystem::EnqueueBlocking(Callable&& callable)
{
    {
        std::unique_lock lock(_blockingMutex);
        _blockingQueue.emplace_back(std::forward<Callable>(callable));
    }
    _blockingCondition.notify_one();
}

template<typename CoroutineJob>
auto JobSystem::EnqueueCoroutine(CoroutineJob&& coroTask)
{
    using ResultType = decltype(coroTask.get_result());
    struct Awaiter
    {
        CoroutineJob _task;

        bool await_ready() const { return false; }
        void await_suspend(std::coroutine_handle<> h)
        {
            // Wrap the coroutine task and continuation in a job
            JobSystem::s_instance->Enqueue([this, h]() mutable
                {
                    _task.start(); // Execute the coroutine until it suspends or completes
                    if (_task.done())
                    {
                        // If done immediately, resume the caller directly
                        h.resume();
                    }
                    else {
                        // Otherwise, store the handle for later resumption
                        _task.set_continuation(h);
                    }
                });
        }
        ResultType await_resume() { return _task.get_result(); }
    };
    return Awaiter{ this, std::forward<CoroutineJob>(coroTask) };
}

template<typename Callable>
auto JobSystem::RunBlockingJob(Callable&& callable)
{
    using ResultType = decltype(callable());
    struct BlockingAwaiter
    {
        Callable callable;

        bool await_ready() const { return false; }
        void await_suspend(std::coroutine_handle<> h)
        {
            JobSystem::s_instance->EnqueueBlocking([this, h]()
                {
                    _result = callable();
                    JobSystem::s_instance->Enqueue([h]() { h.resume(); });
                });
        }
        ResultType await_resume() { return std::move(_result); }

        std::optional<ResultType> _result;
    };
    return BlockingAwaiter{ this, std::forward<Callable>(callable) };
}

/// Basic awaitable for resuming on the job system threads
struct ScheduleOnJobSystem
{
    bool await_ready() const { return false; }
    void await_suspend(std::coroutine_handle<> h) { JobSystem::s_instance->Enqueue([h]() { h.resume(); }); }
    void await_resume() const {}
};