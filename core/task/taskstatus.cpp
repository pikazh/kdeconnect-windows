#include "taskstatus.h"

#include <QCoreApplication>
#include <QHash>

QHash<int, QString> initTaskStatusHashTable()
{
    QHash<int, QString> taskStatusStrings;
    taskStatusStrings.insert(TaskStatus::WaitForStart,
                             QCoreApplication::translate("taskStatus", "Waiting"));
    taskStatusStrings.insert(TaskStatus::ConnectingToPeer,
                             QCoreApplication::translate("taskStatus", "Connecting"));
    taskStatusStrings.insert(TaskStatus::WaitingIncomeConnection,
                             QCoreApplication::translate("taskStatus", "Waiting for peer"));
    taskStatusStrings.insert(TaskStatus::Transfering,
                             QCoreApplication::translate("taskStatus", "Transfering"));

    return taskStatusStrings;
}

QString taskStatusString(int taskStatus)
{
    static QHash<int, QString> taskStatusStrings = initTaskStatusHashTable();
    return taskStatusStrings[taskStatus];
}
