#pragma once

#include "kdeconnectcore_export.h"

#include <QString>

enum TaskStatus {
    WaitForStart = 0,
    ConnectingToPeer,
    WaitingIncomeConnection,
    Transfering,

};

QString KDECONNECTCORE_EXPORT taskStatusString(int taskStatus);
