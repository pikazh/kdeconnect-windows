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
}

Notification::~Notification()
{

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

void Notification::close()
{

}

void Notification::notify()
{
    notifyInterface->notify(this);
}

int Notification::id()
{
    return d->id;
}
