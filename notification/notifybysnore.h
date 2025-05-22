#pragma once

#include "notificationplugin.h"

#include <QHash>
#include <QLocalServer>
#include <QProcess>
#include <QTemporaryDir>

class NotifyBySnore : public NotificationPlugin
{
    Q_OBJECT
public:
    explicit NotifyBySnore(QObject *parent = nullptr);
    virtual ~NotifyBySnore() override;

    virtual void init() override;
    virtual void notify(Notification *n) override;
    virtual void close(int id) override;

    void installAppShortCut();

protected:
    static QString SnoreToastExecPath();

private:
    QLocalServer m_server;
    QTemporaryDir m_iconDir;
    QHash<int, Notification *> m_notifications;
};
