#pragma once

#include "daemoninterface.h"
#include <QQmlEngine>

class DaemonDBusInterface : public OrgKdeKdeconnectDaemonInterface
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    explicit DaemonDBusInterface(QObject *parent = nullptr);

    static QString activatedService();
};
