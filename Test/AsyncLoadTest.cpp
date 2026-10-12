// 메인 스레드 작업 큐의 스레드 안전성과 비동기 로더의 결과 전달 순서/종료를 검증한다.

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>

#include "Core/AsyncLoader.h"
#include "Core/MainThreadQueue.h"

struct QueueRecord
{
    int             producer = 0;
    int             seq      = 0;
    std::thread::id runner   = {};
};

class QueueRecordTask final : public IMainThreadTask
{
public:
    QueueRecordTask(Vector<QueueRecord>* _pOut, int _producer, int _seq)
        : m_pOut(_pOut)
        , m_producer(_producer)
        , m_seq(_seq)
    {
    }

    void Run() override
    {
        m_pOut->push_back({ m_producer, m_seq, std::this_thread::get_id() });
    }

private:
    Vector<QueueRecord>* m_pOut     = nullptr;
    int                  m_producer = 0;
    int                  m_seq      = 0;
};

class QueueProducer final
{
public:
    QueueProducer(MainThreadQueue* _pQueue, Vector<QueueRecord>* _pOut, int _producer, int _count)
        : m_pQueue(_pQueue)
        , m_pOut(_pOut)
        , m_producer(_producer)
        , m_count(_count)
    {
    }

    void Run()
    {
        for (int i = 0; i < m_count; ++i)
        {
            m_pQueue->Submit(std::make_unique<QueueRecordTask>(m_pOut, m_producer, i));
        }
    }

private:
    MainThreadQueue*     m_pQueue   = nullptr;
    Vector<QueueRecord>* m_pOut     = nullptr;
    int                  m_producer = 0;
    int                  m_count    = 0;
};

TEST(MainThreadQueue, ConcurrentProducersRunEachTaskOnceOnDrainingThreadInOrder)
{
    constexpr int kProducers = 8;
    constexpr int kPerThread = 2000;

    MainThreadQueue       queue;
    Vector<QueueRecord>   records;
    Vector<QueueProducer> producers;
    for (int p = 0; p < kProducers; ++p)
    {
        producers.emplace_back(&queue, &records, p, kPerThread);
    }
    Vector<std::thread> threads;
    for (QueueProducer& producer: producers)
    {
        threads.emplace_back(&QueueProducer::Run, &producer);
    }

    const SteadyClock::time_point start = SteadyClock::now();
    while (records.size() < static_cast<size_t>(kProducers * kPerThread) && SteadyClock::now() - start < std::chrono::seconds { 20 })
    {
        queue.DrainAll();
        (void)queue.WaitForTask(1);
    }
    for (std::thread& thread: threads)
    {
        thread.join();
    }
    queue.DrainAll();

    ASSERT_EQ(records.size(), static_cast<size_t>(kProducers * kPerThread));
    Vector<int> next(kProducers, 0);
    for (const QueueRecord& record: records)
    {
        EXPECT_EQ(record.runner, std::this_thread::get_id());
        ASSERT_EQ(record.seq, next[static_cast<size_t>(record.producer)]);
        ++next[static_cast<size_t>(record.producer)];
    }
    for (int p = 0; p < kProducers; ++p)
    {
        EXPECT_EQ(next[static_cast<size_t>(p)], kPerThread);
    }
    EXPECT_EQ(queue.GetPendingCount(), 0u);
}

struct LoadLog
{
    std::mutex              mutex           = {};
    Vector<int>             completeOrder   = {};
    Vector<std::thread::id> completeThreads = {};
    std::atomic<int>        executeOffMain  = 0;
    std::atomic<int>        destroyed       = 0;
    std::thread::id         mainThread      = {};
};

class OrderedLoadJob final : public IAsyncLoadJob
{
public:
    OrderedLoadJob(LoadLog* _pLog, int _index, int _sleepMs)
        : m_pLog(_pLog)
        , m_index(_index)
        , m_sleepMs(_sleepMs)
    {
    }

    ~OrderedLoadJob() override
    {
        m_pLog->destroyed.fetch_add(1);
    }

    void Execute() override
    {
        if (m_pLog->mainThread != std::this_thread::get_id())
        {
            m_pLog->executeOffMain.fetch_add(1);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds { m_sleepMs });
    }

    void Complete() override
    {
        const std::lock_guard<std::mutex> lock { m_pLog->mutex };
        m_pLog->completeOrder.push_back(m_index);
        m_pLog->completeThreads.push_back(std::this_thread::get_id());
    }

private:
    LoadLog* m_pLog    = nullptr;
    int      m_index   = 0;
    int      m_sleepMs = 0;
};

static void RunOrderedLoad_(
    const int _workers)
{
    constexpr int kJobs = 40;

    MainThreadQueue queue;
    AsyncLoader     loader { queue };
    LoadLog         log;
    log.mainThread = std::this_thread::get_id();

    loader.Start(_workers);
    for (int i = 0; i < kJobs; ++i)
    {
        // 뒤쪽 작업일수록 먼저 끝나게 해서 완료 순서가 뒤집히도록 만든다
        loader.Submit(std::make_unique<OrderedLoadJob>(&log, i, (kJobs - i) % 7));
    }
    loader.WaitAll();

    ASSERT_EQ(log.completeOrder.size(), static_cast<size_t>(kJobs));
    for (int i = 0; i < kJobs; ++i)
    {
        EXPECT_EQ(log.completeOrder[static_cast<size_t>(i)], i);
        EXPECT_EQ(log.completeThreads[static_cast<size_t>(i)], log.mainThread);
    }
    EXPECT_EQ(loader.GetSubmittedCount(), static_cast<size_t>(kJobs));
    EXPECT_EQ(loader.GetCompletedCount(), static_cast<size_t>(kJobs));
    if (_workers > 0)
    {
        EXPECT_EQ(log.executeOffMain.load(), kJobs);
    }
    else
    {
        EXPECT_EQ(log.executeOffMain.load(), 0);
    }
}

TEST(AsyncLoader, ResultsAreDeliveredInSubmissionOrderOnMainThread)
{
    RunOrderedLoad_(4);
}

TEST(AsyncLoader, SynchronousFallbackKeepsOrder)
{
    RunOrderedLoad_(0);
}

class ShutdownScenario final
{
public:
    void Run()
    {
        {
            MainThreadQueue queue;
            AsyncLoader     loader { queue };
            LoadLog         log;
            log.mainThread = std::this_thread::get_id();
            loader.Start(2);
            for (int i = 0; i < kJobs; ++i)
            {
                loader.Submit(std::make_unique<OrderedLoadJob>(&log, i, 5));
            }
            loader.Shutdown();
            m_destroyed = log.destroyed.load();
        }
        m_bDone.store(true);
    }

    static constexpr int kJobs = 60;

    std::atomic<bool> m_bDone     = false;
    int               m_destroyed = 0;
};

TEST(AsyncLoader, ShutdownWithPendingJobsDoesNotDeadlockOrLeak)
{
    ShutdownScenario scenario;
    std::thread      runner { &ShutdownScenario::Run, &scenario };

    const SteadyClock::time_point start = SteadyClock::now();
    while (!scenario.m_bDone.load() && SteadyClock::now() - start < std::chrono::seconds { 10 })
    {
        std::this_thread::sleep_for(std::chrono::milliseconds { 5 });
    }
    if (!scenario.m_bDone.load())
    {
        runner.detach();
        FAIL() << "Shutdown deadlocked";
    }
    runner.join();
    EXPECT_EQ(scenario.m_destroyed, ShutdownScenario::kJobs);
}
