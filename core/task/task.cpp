#include "task.h"
#include "core_debug.h"

#include <QString>
#include <QTextStream>

Task::Task(QObject *parent)
    : QObject{parent}
{
    m_uid = QUuid::createUuid();
}

void Task::setProgress(qint64 current, qint64 total)
{
    if (current != m_currentProgress || total != m_totalProgress) {
        m_currentProgress = current;
        m_totalProgress = total;

        emit progress(m_currentProgress, m_totalProgress);
    }
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
        qDebug(KDECONNECT_CORE) << "Task" << describe() << "restarting after succeeding.";
        break;
    case Task::State::Failed:
        qDebug(KDECONNECT_CORE) << "Task" << describe() << "restarting after failing.";
        break;
    case Task::State::Aborted:
        qDebug(KDECONNECT_CORE) << "Task" << describe()
                                << "restarting after being aborted by user.";
        break;
    }

    setState(Task::State::Running);
    emit started();
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

    emit succeeded();
    emit finished();
}

void Task::emitAborted()
{
    if (!isRunning()) {
        qCritical(KDECONNECT_CORE)
            << "Task" << describe() << "call emitAborted while not running!!";
        return;
    }

    setState(Task::State::Aborted);

    emit aborted();
    emit finished();
}

void Task::emitFailed(QString failReason)
{
    if (!isRunning()) {
        qCritical(KDECONNECT_CORE) << "Task" << describe() << "call emitFailed while not running!!";
        return;
    }

    setState(Task::State::Failed);

    emit failed(failReason);
    emit finished();
}
