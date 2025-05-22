#pragma once

#include <QTextDocumentFragment>

class Notification;

class NotificationPlugin : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual void init() {}
    virtual void notify(Notification *notification) = 0;
    virtual void close(int id) { emit finished(id); }

protected:
    static inline QString stripRichText(const QString &s)
    {
        return QTextDocumentFragment::fromHtml(s).toPlainText();
    }

Q_SIGNALS:
    void actionInvoked(int id, const QString &actionId);
    void finished(int id);
};
