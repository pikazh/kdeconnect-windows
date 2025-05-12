#ifndef NOTIFYBYSNORE_H
#define NOTIFYBYSNORE_H

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
    virtual void notify(Notification *notification) override;
    virtual void close(Notification *notification) override;

    void installAppShortCut();

protected Q_SLOTS:
    void notifyDeferred(Notification *notification);

protected:
    static QString SnoreToastExecPath();

private:
    QLocalServer m_server;
    QTemporaryDir m_iconDir;
    QHash<int, Notification *> m_notifications;
};

#endif // NOTIFYBYSNORE_H
