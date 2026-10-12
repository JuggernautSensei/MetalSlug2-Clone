#include "Core/AsyncLoader.h"

#include <algorithm>

IAsyncLoadJob::~IAsyncLoadJob() = default;

class AsyncLoader::ExecutedNotice final : public IMainThreadTask
{
public:
    ExecutedNotice(AsyncLoader* _pLoader, size_t _index);

    void Run() override;

private:
    AsyncLoader* m_pLoader = nullptr;
    size_t       m_index   = 0;
};

AsyncLoader::ExecutedNotice::ExecutedNotice(
    AsyncLoader* const _pLoader,
    const size_t       _index)
    : m_pLoader(_pLoader)
    , m_index(_index)
{
}

void AsyncLoader::ExecutedNotice::Run()
{
    m_pLoader->OnExecuted_(m_index);
}

AsyncLoader::AsyncLoader(
    MainThreadQueue& _mainQueue)
    : m_mainQueue(_mainQueue)
{
}

AsyncLoader::~AsyncLoader()
{
    Shutdown();
}

int AsyncLoader::ChooseWorkerCount()
{
    const int hardware = static_cast<int>(std::thread::hardware_concurrency());
    return std::clamp(hardware - 1, 1, 8);
}

void AsyncLoader::Start(
    const int _workerCount)
{
    Shutdown();

    const int count = std::max(0, _workerCount);
    {
        const std::lock_guard<std::mutex> lock { m_mutex };
        m_bStop     = false;
        m_completed = 0;
        m_work.clear();
    }
    m_entries.clear();
    m_entryBase = 0;
    m_submitted = 0;
    m_window    = static_cast<size_t>(count);   // 끝났지만 아직 전달 안 된 결과 수의 상한 (메모리)
    m_bStopped  = false;
    for (int i = 0; i < count; ++i)
    {
        m_workers.emplace_back(&AsyncLoader::WorkerMain_, this);
    }
}

void AsyncLoader::Submit(
    Scoped<IAsyncLoadJob> _pJob)
{
    if (!_pJob || m_bStopped)
    {
        return;
    }

    if (m_workers.empty())
    {
        _pJob->Execute();
        _pJob->Complete();
        ++m_submitted;
        const std::lock_guard<std::mutex> lock { m_mutex };
        ++m_completed;
        return;
    }

    const size_t index = m_submitted++;
    Entry        entry;
    entry.pJob = std::move(_pJob);
    m_entries.push_back(std::move(entry));
    {
        const std::lock_guard<std::mutex> lock { m_mutex };
        m_work.push_back({ m_entries.back().pJob.GetPtr(), index });
    }
    m_workCv.notify_one();
}

void AsyncLoader::WaitAll()
{
    while (!m_bStopped && GetCompletedCount() < m_submitted)
    {
        m_mainQueue.DrainAll();
        if (GetCompletedCount() < m_submitted)
        {
            (void)m_mainQueue.WaitForTask(20);
        }
    }
}

void AsyncLoader::Shutdown()
{
    if (m_bStopped)
    {
        return;
    }
    m_bStopped = true;
    {
        const std::lock_guard<std::mutex> lock { m_mutex };
        m_bStop = true;
        m_work.clear();
    }
    m_workCv.notify_all();
    for (std::thread& worker: m_workers)
    {
        worker.join();
    }
    m_workers.clear();

    // 워커가 남긴 완료 통지가 큐에 있어도 m_bStopped 라 아무것도 하지 않는다. 큐를 비워 통지를 치운다.
    m_mainQueue.DrainAll();
    m_entries.clear();
}

int AsyncLoader::GetWorkerCount() const
{
    return static_cast<int>(m_workers.size());
}

size_t AsyncLoader::GetSubmittedCount() const
{
    return m_submitted;
}

size_t AsyncLoader::GetCompletedCount() const
{
    const std::lock_guard<std::mutex> lock { m_mutex };
    return m_completed;
}

bool AsyncLoader::CanTakeWork_() const
{
    return !m_work.empty() && m_work.front().index < m_completed + m_window;
}

void AsyncLoader::WorkerMain_()
{
    for (;;)
    {
        WorkItem item;
        {
            std::unique_lock<std::mutex> lock { m_mutex };
            while (!m_bStop && !CanTakeWork_())
            {
                m_workCv.wait(lock);
            }
            if (m_bStop)
            {
                return;
            }
            item = m_work.front();
            m_work.pop_front();
        }

        item.pJob->Execute();
        m_mainQueue.Submit(Scoped<IMainThreadTask> { new ExecutedNotice(this, item.index) });
    }
}

void AsyncLoader::OnExecuted_(
    const size_t _index)
{
    if (m_bStopped || _index < m_entryBase || _index - m_entryBase >= m_entries.size())
    {
        return;
    }
    m_entries[_index - m_entryBase].bExecuted = true;
    Deliver_();
}

void AsyncLoader::Deliver_()
{
    bool bAdvanced = false;
    for (;;)
    {
        const size_t next = GetCompletedCount();
        if (next < m_entryBase || next - m_entryBase >= m_entries.size() || !m_entries[next - m_entryBase].bExecuted)
        {
            break;
        }

        Scoped<IAsyncLoadJob> pJob = std::move(m_entries[next - m_entryBase].pJob);
        pJob->Complete();   // 여기서 Submit 이 불려도 m_entries 를 다시 인덱스로 읽으므로 안전하다
        pJob.Reset();
        {
            const std::lock_guard<std::mutex> lock { m_mutex };
            ++m_completed;
        }
        bAdvanced = true;
    }
    if (bAdvanced)
    {
        if (GetCompletedCount() - m_entryBase == m_entries.size())
        {
            m_entryBase += m_entries.size();
            m_entries.clear();
        }
        m_workCv.notify_all();
    }
}
