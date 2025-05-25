#pragma once

#include <QObject>
#include <QUuid>

#include "QObjectPtr.h"
#include "kdeconnectcore_export.h"

class KDECONNECTCORE_EXPORT Task : public QObject, public QEnableSharedFromThis<Task>
{
    friend class TaskScheduler;
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<Task>;
    enum class State { Inactive, Running, Succeeded, Failed, Aborted };
    Q_ENUM(State)

    explicit Task(QObject *parent = nullptr);
    virtual ~Task() override = default;

    qint64 currentProgress() const { return m_currentProgress; }
    qint64 totalProgress() const { return m_totalProgress; }

    State state() const { return m_state; }
    bool isRunning() const;
    bool isFinished() const;
    bool isSuccessful() const;
    bool isFailed() const;
    bool isAborted() const;

    QUuid uid() const { return m_uid; }
    QString describe();

protected:
    void setProgress(qint64 current, qint64 total);
    void setState(Task::State state) { m_state = state; }
    virtual bool canAbort() const { return m_canAbort; }
    void setAbortable(bool canAbort) { m_canAbort = canAbort; }

protected:
    // you should not call these functions by yourself
    bool start();
    bool abort();
    virtual void executeTask() = 0;
    virtual void onAbort() { emitAborted(); }

protected Q_SLOTS:
    void emitSucceeded();
    void emitAborted();
    void emitFailed(const QString &failReason);

Q_SIGNALS:
    void started();
    void progress(qint64 current, qint64 total);
    void finished();
    void succeeded();
    void failed(const QString &reason);
    void aborted();

private:
    State m_state = State::Inactive;
    qint64 m_currentProgress = 0;
    qint64 m_totalProgress = -1;
    bool m_canAbort = true;
    QUuid m_uid;
};

class KDECONNECTCORE_EXPORT ITaskScheduler : public QObject
{
    Q_OBJECT
public:
    explicit ITaskScheduler(QObject *parent = nullptr)
        : QObject(parent)
    {}
    virtual ~ITaskScheduler() override = default;

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool addTask(Task::Ptr task) = 0;
    virtual bool removeTask(Task::Ptr task) = 0;
};
