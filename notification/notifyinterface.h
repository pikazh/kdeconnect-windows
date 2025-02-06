#ifndef NOTIFYINTERFACE_H
#define NOTIFYINTERFACE_H

#include <QObject>
#include <QTextDocumentFragment>

class Notification;

class NotifyInterface: public QObject
{
    Q_OBJECT
public:
    explicit NotifyInterface(QObject *parent = nullptr)
        : QObject(parent)
    {

    }

    virtual void notify(Notification *notification) = 0;
    virtual void close(Notification *notification) = 0;

protected:
    static inline QString stripRichText(const QString &s)
    {
        return QTextDocumentFragment::fromHtml(s).toPlainText();
    }

Q_SIGNALS:
    void actionInvoked(Notification *notification, const QString &action);
    void finished(Notification *notification);
};

#endif // NOTIFYINTERFACE_H
