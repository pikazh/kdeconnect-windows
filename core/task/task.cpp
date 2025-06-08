#include "task.h"
#include "core_debug.h"

#include <QString>
#include <QTextStream>

Task::Task(QObject *parent)
    : QObject{parent}
{
    m_uid = QUuid::createUuid();
}

Task::~Task()
{
    qDebug(KDECONNECT_CORE) << "Task" << describe() << "now destroy";
}

void Task::setProgress(qint64 current, qint64 total)
{
    if (current != m_currentProgress || total != m_totalProgress) {
        m_currentProgress = current;
        m_totalProgress = total;

        Q_EMIT progress(m_currentProgress, m_totalProgress);
    }
}

void Task::setTaskStatus(int taskStatus)
{
    if (m_taskStatus != taskStatus) {
        m_taskStatus = taskStatus;
        Q_EMIT statusChanged(m_taskStatus);
    }
}

bool Task::isInactive() const
{
    return m_state == Task::State::Inactive;
}

bool Task::isRunning() const
{
    return m_state == Task::State::Running;
}

bool Task::isFinished() const
{
    return m_state != Task::State::Running && m_state != Task::State::Inactive;
}

bool Task::isSuccessful() const
{
    return m_state == Task::State::Succeeded;
}

bool Task::isFailed() const
{
    return m_state == Task::State::Failed;
}

bool Task::isAborted() const
{
    return m_state == Task::State::Aborted;
}

QString Task::failedReasson() const
{
    return m_failedReason;
}

bool Task::start()
{
    auto curentState = state();
    switch (curentState) {
    case Task::State::Inactive:
        qDebug(KDECONNECT_CORE) << "Task" << describe() << "starting for the first time";
        break;
    case Task::State::Running:
        qCritical(KDECONNECT_CORE)
            << "Task" << describe() << "is already running, but app tried to start it again.";
        return false;
    case Task::State::Succeeded:
        qCWarning(KDECONNECT_CORE) << "Task" << describe() << "restarting after succeeding.";
        break;
    case Task::State::Failed:
        qCWarning(KDECONNECT_CORE) << "Task" << describe() << "restarting after failing.";
        break;
    case Task::State::Aborted:
        qCWarning(KDECONNECT_CORE)
            << "Task" << describe() << "restarting after being aborted by user.";
        break;
    }

    setState(Task::State::Running);
    Q_EMIT started();
    return true;
}

bool Task::abort()
{
    if (isRunning() && canAbort()) {
        onAbort();
        return true;
    } else {
        return false;
    }
}

QString Task::describe()
{
    QString outStr;
    QTextStream out(&outStr);
    out << metaObject()->className() << QChar('(');
    auto name = objectName();
    if (name.isEmpty()) {
        out << QString("0x%1").arg(reinterpret_cast<quintptr>(this), 0, 16);
    } else {
        out << name;
    }
    out << " ID: " << m_uid.toString(QUuid::WithoutBraces);
    out << QChar(')');
    out.flush();
    return outStr;
}

void Task::emitSucceeded()
{
    if (!isRunning()) {
        qCritical(KDECONNECT_CORE)
            << "Task" << describe() << "call emitSucceeded while not running!!";
        return;
    }

    setState(Task::State::Succeeded);

    Q_EMIT succeeded();
    Q_EMIT finished();
}

void Task::emitAborted()
{
    if (!isRunning()) {
        qCritical(KDECONNECT_CORE)
            << "Task" << describe() << "call emitAborted while not running!!";
        return;
    }

    setState(Task::State::Aborted);

    Q_EMIT aborted();
    Q_EMIT finished();
}

void Task::emitFailed(const QString &failReason)
{
    if (!isRunning()) {
        qCritical(KDECONNECT_CORE) << "Task" << describe() << "call emitFailed while not running!!";
        return;
    }

    setState(Task::State::Failed);

    Q_EMIT failed(failReason);
    Q_EMIT finished();
}
