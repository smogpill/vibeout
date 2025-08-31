// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "JobSystem.h"

JobSystem::JobSystem(uint numThreads) 
{
    // Create worker threads
    _threads.reserve(numThreads);
    for (uint i = 0; i < numThreads; ++i)
    {
        _threads.emplace_back(&JobSystem::WorkerThread, this);
    }

    // Create blocking I/O thread
    _blockingThread = std::jthread([this](std::stop_token st)
        {
            while (!st.stop_requested())
            {
                Job job;
                {
                    std::unique_lock lock(_blockingMutex);
                    _blockingCondition.wait(lock, [this, &st]
                        {
                            return !_blockingQueue.empty() || st.stop_requested();
                        });
                    if (st.stop_requested()) break;
                    job = std::move(_blockingQueue.front());
                    _blockingQueue.pop_front();
                }
                job();
            }
        });
}

JobSystem::~JobSystem()
{
    Stop();
}

void JobSystem::WorkerThread()
{
    while (_running)
    {
        Job job;
        {
            std::unique_lock lock(_queueMutex);
            _condition.wait(lock, [this]
                {
                    return !_jobQueue.empty() || !_running;
                });

            if (!_running && _jobQueue.empty())
                break;

            job = std::move(_jobQueue.front());
            _jobQueue.pop_front();
        }
        job();
    }
}

void JobSystem::Stop()
{
    if (!_running.exchange(false))
        return;

    _condition.notify_all();
    _blockingCondition.notify_all();

    for (auto& thread : _threads)
    {
        if (thread.joinable())
            thread.join();
    }

    _blockingThread.request_stop();
    _blockingCondition.notify_all();
    if (_blockingThread.joinable())
    {
        _blockingThread.join();
    }
}
