#include "Core/MainThreadQueue.h"

#include <chrono>

IMainThreadTask::~IMainThreadTask() = default;

MainThreadQueue::MainThreadQueue() = default;

MainThreadQueue::~MainThreadQueue() = default;

void MainThreadQueue::Submit(
    Scoped<IMainThreadTask> _pTask)
{
    if (!_pTask)
    {
        return;
    }
    {
        const std::lock_guard<std::mutex> lock { m_mutex };
        m_tasks.push_back(std::move(_pTask));
    }
    m_cv.notify_one();
}

size_t MainThreadQueue::DrainAll()
{
    Vector<Scoped<IMainThreadTask>> batch;
    {
        const std::lock_guard<std::mutex> lock { m_mutex };
        batch.swap(m_tasks);
    }
    for (const Scoped<IMainThreadTask>& pTask: batch)
    {
        pTask->Run();
    }
    return batch.size();
}

bool MainThreadQueue::WaitForTask(
    const int _timeoutMs)
{
    std::unique_lock<std::mutex> lock { m_mutex };
    if (m_tasks.empty())
    {
        m_cv.wait_for(lock, std::chrono::milliseconds { _timeoutMs });
    }
    return !m_tasks.empty();
}

size_t MainThreadQueue::GetPendingCount() const
{
    const std::lock_guard<std::mutex> lock { m_mutex };
    return m_tasks.size();
}
