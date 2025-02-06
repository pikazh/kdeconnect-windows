#ifndef NOTIFYBYSNORE_H
#define NOTIFYBYSNORE_H

#include "notifyinterface.h"

#include <QLocalServer>
#include <QProcess>
#include <QTemporaryDir>

class NotifyBySnore: public NotifyInterface
{
    Q_OBJECT
public:
    explicit NotifyBySnore(QObject *parent = nullptr);
    virtual ~NotifyBySnore();

    virtual void notify(Notification *notification) override;
    virtual void close(Notification *notification) override;

    void installAppShortCut();

protected:
    void notifyDeferred(Notification *notification);
    static QString SnoreToastExecName();

private:
    QLocalServer m_server;
    QTemporaryDir m_iconDir;
    Notification *m_notification;
    static int instanceCount;
};

#endif // NOTIFYBYSNORE_H
