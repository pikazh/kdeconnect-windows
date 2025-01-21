#pragma once

#include "daemoninterface.h"

class DaemonDBusInterface : public OrgKdeKdeconnectDaemonInterface
{
    Q_OBJECT

public:
    explicit DaemonDBusInterface(QObject *parent = nullptr);

    static QString activatedService();
};
