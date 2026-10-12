#pragma once
#include <condition_variable>
#include <memory>
#include <mutex>

#include "Base/Types.h"

class IMainThreadTask
{
public:
    virtual ~IMainThreadTask();

    virtual void Run() = 0;
};

// 아무 스레드에서나 Submit 할 수 있고, DrainAll 을 부른 (메인) 스레드에서만 Run 된다. 한 생산자 안에서는 FIFO.
class MainThreadQueue final
{
public:
    MainThreadQueue();
    ~MainThreadQueue();

    MainThreadQueue(const MainThreadQueue&)            = delete;
    MainThreadQueue& operator=(const MainThreadQueue&) = delete;

    void Submit(std::unique_ptr<IMainThreadTask> _pTask);

    // 지금 쌓여 있는 작업을 한 번 비운다 (비우는 도중 새로 들어온 작업은 다음 호출에서). 실행한 개수를 돌려준다.
    size_t DrainAll();

    // 작업이 들어올 때까지 최대 _timeoutMs 기다린다. 큐가 비어있지 않으면 true.
    [[nodiscard]] bool WaitForTask(int _timeoutMs);

    [[nodiscard]] size_t GetPendingCount() const;

private:
    mutable std::mutex                       m_mutex = {};
    std::condition_variable                  m_cv    = {};
    Vector<std::unique_ptr<IMainThreadTask>> m_tasks = {};
};
