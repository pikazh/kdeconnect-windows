#include "notification.h"
#include "notifybysnore.h"

#include <QList>

struct NotificationActionPrivate
{
    QString label;
    QString id;
};

struct NotificationPrivate
{
    int id = 0;
    QString title;
    QString text;
    QPixmap pixmap;
    QList<NotificationAction*> actions;
    bool closed = false;
    bool autoDestroy = true;
    int actionIdCounter = 0;

    static int notificationIdCounter;
};

int NotificationPrivate::notificationIdCounter = 0;

NotificationAction::NotificationAction(QObject *parent)
    : QObject(parent)
    , d(new NotificationActionPrivate)
{
}

NotificationAction::NotificationAction(const QString &label, QObject *parent)
    : QObject(parent)
    , d(new NotificationActionPrivate)
{
    setLabel(label);
}

NotificationAction::~NotificationAction()
{

}

QString NotificationAction::label() const
{
    return d->label;
}

void NotificationAction::setLabel(const QString &label)
{
    if(d->label != label)
    {
        d->label = label;
    }
}

void NotificationAction::setId(const QString &id)
{
    d->id = id;
}

QString NotificationAction::id() const
{
    return d->id;
}

Notification::Notification(QObject *parent)
    : QObject(parent)
    , d(new NotificationPrivate)
{
    d->id = ++d->notificationIdCounter;
    notifyInterface = new NotifyBySnore(this);

    connect(notifyInterface, &NotifyInterface::actionInvoked, this, &Notification::onActionInvoked);
    connect(notifyInterface, &NotifyInterface::finished, this, &Notification::onNotifyFinished);
}

Notification::~Notification()
{
    close();
}

QString Notification::title() const
{
    return d->title;
}

void Notification::setTitle(const QString &title)
{
    if(d->title != title)
    {
        d->title = title;
    }
}

QString Notification::text() const
{
    return d->text;
}

void Notification::setText(const QString &text)
{
    if(d->text != text)
    {
        d->text = text;
    }
}

QPixmap Notification::pixmap() const
{
    return d->pixmap;
}

void Notification::setPixmap(const QPixmap &pixmap)
{
    d->pixmap = pixmap;
}

NotificationAction *Notification::addAction(const QString &label)
{
    NotificationAction *action = new NotificationAction(label);
    action->setId(QString::number(++d->actionIdCounter));
    d->actions << action;
    return action;
}

void Notification::clearActions()
{
    qDeleteAll(d->actions);
    d->actions.clear();
}

void Notification::setAutoDestroy(bool autoDestroy)
{
    d->autoDestroy = autoDestroy;
}

bool Notification::autoDestroy() const
{
    return d->autoDestroy;
}

QList<NotificationAction *> Notification::actions() const
{
    return d->actions;
}

void Notification::close()
{
    if(!d->closed)
    {
        notifyInterface->close(this);
    }
}

void Notification::notify()
{
    notifyInterface->notify(this);
}

void Notification::onActionInvoked(Notification *notification, const QString &actionLabel)
{
    Q_ASSERT(notification == this);
    for(auto action : actions())
    {
        if(action->label() == actionLabel)
        {
            Q_EMIT action->activated();
            break;
        }
    }
}

void Notification::onNotifyFinished(Notification *notification)
{
    Q_ASSERT(notification == this);
    d->closed = true;
    if(autoDestroy())
    {
        deleteLater();
    }
}

int Notification::id()
{
    return d->id;
}
