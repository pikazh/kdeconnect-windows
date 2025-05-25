#include "taskscheduler.h"
#include "core_debug.h"

TaskScheduler::TaskScheduler(int maxConcurrentNum, QObject *parent)
    : ITaskScheduler(parent)
    , m_maxConcurrentTaskNum(maxConcurrentNum)
    , m_state(State::Stopped)
{}

bool TaskScheduler::start()
{
    if (state() != State::Stopped) {
        return false;
    }

    setState(State::Running);
    scheduleTask();
    return true;
}

bool TaskScheduler::stop()
{
    if (state() != State::Running) {
        return false;
    }

    setState(State::WaitingForStop);

    bool allAborted = true;
    for (auto task : m_doing) {
        allAborted &= task->abort();
    }

    return allAborted;
}

bool TaskScheduler::addTask(Task::Ptr task)
{
    Task *taskPtr = task.get();
    if (m_taskIndexs.contains(taskPtr)) {
        qWarning(KDECONNECT_CORE) << "Task" << task->describe() << "already added to schedule";
        return false;
    } else {
        m_taskIndexs.insert(taskPtr);
        m_queue.append(task);
        scheduleTask();
        return true;
    }
}

bool TaskScheduler::removeTask(Task::Ptr task)
{
    Task *taskPtr = task.get();
    if (m_taskIndexs.contains(taskPtr)) {
        m_taskIndexs.remove(taskPtr);

        if (removeTaskFromScheduleQueue(taskPtr)) {
            return true;
        } else if (m_doing.remove(taskPtr)) {
            QObject::disconnect(taskPtr, nullptr, this, nullptr);
            taskPtr->abort();
            if (state() == State::Running) {
                scheduleTask();
            }
            return true;
        } else if (m_done.remove(taskPtr)) {
            m_failed.remove(taskPtr);
            m_succeeded.remove(taskPtr);
            m_aborted.remove(taskPtr);
            return true;
        } else {
            Q_ASSERT(0);
            qCritical(KDECONNECT_CORE)
                << "Task" << task->describe() << "is contained in index, but not any container.";
            return false;
        }
    }

    return false;
}

void TaskScheduler::scheduleTask()
{
    QMetaObject::invokeMethod(this, &TaskScheduler::executeNextTask, Qt::QueuedConnection);
}

void TaskScheduler::runTask(Task::Ptr task)
{
    Task *taskPtr = task.get();

    QObject::connect(taskPtr, &Task::started, this, [this, task]() { onTaskStarted(task); });
    QObject::connect(taskPtr, &Task::succeeded, this, [this, task]() { onTaskSucceeded(task); });
    QObject::connect(taskPtr, &Task::failed, this, [this, task](const QString &reason) {
        onTaskFailed(task, reason);
    });
    QObject::connect(taskPtr, &Task::aborted, this, [this, task]() { onTaskAbortd(task); });
    QObject::connect(taskPtr, &Task::progress, this, [this, task](qint64 current, qint64 total) {
        onTaskProgress(task, current, total);
    });

    task->start();
    task->executeTask();
}

bool TaskScheduler::stopCheck()
{
    auto stat = state();
    if (stat == State::Stopped) {
        return true;
    } else if (stat == State::WaitingForStop) {
        if (m_doing.size() == 0) {
            setState(State::Stopped);
        }

        return true;
    }

    return false;
}

bool TaskScheduler::removeTaskFromScheduleQueue(Task *taskPtr)
{
    for (auto it = m_queue.begin(); it != m_queue.end();) {
        if (it->get() == taskPtr) {
            m_queue.erase(it);
            return true;
        } else {
            ++it;
        }
    }

    return false;
}

void TaskScheduler::onTaskStarted(Task::Ptr task)
{
    emit taskStarted(task);
}

void TaskScheduler::onTaskFailed(Task::Ptr task, const QString &msg)
{
    onTaskFinished(task, Task::State::Failed, msg);
}

void TaskScheduler::onTaskSucceeded(Task::Ptr task)
{
    onTaskFinished(task, Task::State::Succeeded);
}

void TaskScheduler::onTaskAbortd(Task::Ptr task)
{
    onTaskFinished(task, Task::State::Aborted);
}

void TaskScheduler::onTaskFinished(Task::Ptr task, Task::State state, const QString &msg)
{
    Task *taskPtr = task.get();
    m_doing.remove(taskPtr);
    m_done.insert(taskPtr, task);

    QObject::disconnect(taskPtr, nullptr, this, nullptr);

    switch (state) {
    case Task::State::Aborted:
        m_aborted.insert(taskPtr, task);
        emit taskAborted(task);
        break;
    case Task::State::Failed:
        m_failed.insert(taskPtr, task);
        emit taskFailed(task, msg);
        break;
    case Task::State::Succeeded:
        m_succeeded.insert(taskPtr, task);
        emit taskSucceeded(task);
        break;
    default:
        qCritical(KDECONNECT_CORE) << "onTaskFinished was call with improper task state" << state;
        break;
    }

    emit taskFinished(task);
    emit scheduleProgress(m_doing.size(), totalTaskNumber());

    if (!stopCheck()) {
        scheduleTask();
    }
}

void TaskScheduler::onTaskProgress(Task::Ptr task, qint64 currentProgress, qint64 totalProgress)
{
    emit taskProgress(task, currentProgress, totalProgress);
}

void TaskScheduler::executeNextTask()
{
    if (state() != State::Running) {
        return;
    }

    if (m_doing.size() >= maxConcurrentTaskNum()) {
        return;
    }

    if (!m_queue.isEmpty()) {
        auto task = m_queue.takeFirst();
        m_doing.insert(task.get(), task);
        runTask(task);

        scheduleTask();
    }
}
