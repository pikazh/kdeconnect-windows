#pragma once

#include "kdeconnectcore_export.h"
#include "task.h"

#include <QHash>
#include <QList>
#include <QSet>

class KDECONNECTCORE_EXPORT TaskScheduler : public ITaskScheduler
{
    Q_OBJECT
public:
    enum class State {
        Stopped,
        Running,
        WaitingForStop,
    };
    TaskScheduler(int maxConcurrentNum = 10, QObject *parent = nullptr);
    virtual ~TaskScheduler() override = default;

    virtual bool start() override;
    virtual bool stop() override;
    virtual bool addTask(Task::Ptr task) override;
    virtual bool removeTask(Task::Ptr task) override;

protected:
    void setState(State state) { m_state = state; }
    State state() { return m_state; }
    int maxConcurrentTaskNum() { return m_maxConcurrentTaskNum; }
    int totalTaskNumber() { return m_doing.size() + m_done.size() + m_queue.size(); }

    void scheduleTask();
    void runTask(Task::Ptr task);
    bool stopCheck();
    bool removeTaskFromScheduleQueue(Task *task);

    void onTaskStarted(Task::Ptr task);
    void onTaskFailed(Task::Ptr task, const QString &msg);
    void onTaskSucceeded(Task::Ptr task);
    void onTaskAbortd(Task::Ptr task);
    void onTaskFinished(Task::Ptr task, Task::State state, const QString &msg = QString());
    void onTaskProgress(Task::Ptr task, qint64 currentProgress, qint64 totalProgress);

protected Q_SLOTS:
    void executeNextTask();

Q_SIGNALS:
    void taskStarted(Task::Ptr task);
    void taskFailed(Task::Ptr task, const QString &reason);
    void taskSucceeded(Task::Ptr task);
    void taskAborted(Task::Ptr task);
    void taskFinished(Task::Ptr task);
    void taskProgress(Task::Ptr task, qint64 currentProgress, qint64 totalProgress);
    void scheduleProgress(int currentProgress, int totalProgress);

private:
    int m_maxConcurrentTaskNum;
    State m_state;

    QList<Task::Ptr> m_queue;
    QHash<Task *, Task::Ptr> m_doing;
    QHash<Task *, Task::Ptr> m_done;

    QHash<Task *, Task::Ptr> m_succeeded;
    QHash<Task *, Task::Ptr> m_failed;
    QHash<Task *, Task::Ptr> m_aborted;

    QSet<Task *> m_taskIndexs;
};
