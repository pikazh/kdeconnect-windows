#pragma once

#include <QObject>
#include <QTextDocumentFragment>

class Notification;

class NotificationPlugin : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual void init() {}
    virtual void notify(Notification *notification) = 0;
    virtual void close(Notification *notification) { emit finished(notification); }

protected:
    static inline QString stripRichText(const QString &s)
    {
        return QTextDocumentFragment::fromHtml(s).toPlainText();
    }

Q_SIGNALS:
    void actionInvoked(Notification *notification, const QString &actionId);
    void finished(Notification *notification);
};
