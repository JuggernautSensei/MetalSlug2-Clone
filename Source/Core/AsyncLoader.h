#pragma once
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

#include "Base/Types.h"
#include "Core/MainThreadQueue.h"

class IAsyncLoadJob
{
public:
    virtual ~IAsyncLoadJob();

    // 워커 스레드에서 실행된다 (디코드 등 CPU 작업만. SDL 렌더러 호출/라이브러리 등록 금지).
    virtual void Execute() = 0;

    // 메인 스레드에서 제출 순서대로 호출된다 (텍스처 생성, 등록).
    virtual void Complete() = 0;
};

// 워커 풀. Start/Submit/WaitAll/Shutdown 은 메인 스레드에서만 부른다.
// 워커 수가 0 이면 Submit 이 그 자리에서 Execute + Complete 를 한다 (동기 폴백: --no-async-load).
class AsyncLoader final
{
public:
    explicit AsyncLoader(MainThreadQueue& _mainQueue);
    ~AsyncLoader();

    AsyncLoader(const AsyncLoader&)            = delete;
    AsyncLoader& operator=(const AsyncLoader&) = delete;

    [[nodiscard]] static int ChooseWorkerCount();

    void Start(int _workerCount);
    void Submit(Scoped<IAsyncLoadJob> _pJob);
    void WaitAll();

    // 워커를 멈추고 합친다. 아직 Complete 되지 않은 작업은 버린다.
    void Shutdown();

    [[nodiscard]] int    GetWorkerCount() const;
    [[nodiscard]] size_t GetSubmittedCount() const;
    [[nodiscard]] size_t GetCompletedCount() const;

private:
    class ExecutedNotice;

    struct Entry
    {
        Scoped<IAsyncLoadJob> pJob      = nullptr;
        bool                  bExecuted = false;
    };

    struct WorkItem
    {
        IAsyncLoadJob* pJob  = nullptr;
        size_t         index = 0;
    };

    void               WorkerMain_();
    void               OnExecuted_(size_t _index);
    void               Deliver_();
    [[nodiscard]] bool CanTakeWork_() const;

    MainThreadQueue&    m_mainQueue;
    Vector<std::thread> m_workers  = {};
    size_t              m_window   = 0;
    bool                m_bStopped = true;

    // 메인 스레드 전용
    Vector<Entry> m_entries   = {};
    size_t        m_entryBase = 0;
    size_t        m_submitted = 0;

    // m_mutex 로 보호
    mutable std::mutex      m_mutex     = {};
    std::condition_variable m_workCv    = {};
    std::deque<WorkItem>    m_work      = {};
    size_t                  m_completed = 0;
    bool                    m_bStop     = false;
};
