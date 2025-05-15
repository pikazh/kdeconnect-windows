#include "notification.h"
#include "notificationmanager.h"

#include <QList>

struct NotificationActionPrivate
{
    QString text;
    QString id;
};

struct NotificationPrivate
{
    int id = -1;
    int ref = 0;
    QString title;
    QString text;
    QPixmap pixmap;
    QString iconName;
    QList<NotificationAction*> actions;
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

NotificationAction::NotificationAction(const QString &text, QObject *parent)
    : QObject(parent)
    , d(new NotificationActionPrivate)
{
    setText(text);
}

NotificationAction::~NotificationAction()
{
    
}

QString NotificationAction::text() const
{
    return d->text;
}

void NotificationAction::setText(const QString &text)
{
    if (d->text != text) {
        d->text = text;
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

QString Notification::standardEventToIconName(Notification::StandardEvent event)
{
    QString iconName;
    switch (event) {
    case Notification::StandardEvent::Warning:
        iconName = QStringLiteral("data-warning");
        break;
    case Notification::StandardEvent::Error:
        iconName = QStringLiteral("data-error");
        break;
    case Notification::StandardEvent::Catastrophe:
        iconName = QStringLiteral("data-error");
        break;
    case Notification::StandardEvent::Notification: // fall through
    default:
        iconName = QStringLiteral("data-information");
        break;
    }
    return iconName;
}

Notification::Notification(QObject *parent)
    : QObject(parent)
    , d(new NotificationPrivate)
{
    d->id = d->notificationIdCounter++;
}

Notification::Notification(StandardEvent eventId, QObject *parent)
    : QObject(parent)
    , d(new NotificationPrivate)
{
    d->id = d->notificationIdCounter++;
    setIconName(standardEventToIconName(eventId));
}

Notification::~Notification()
{
    clearActions();
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

QString Notification::iconName() const
{
    return d->iconName;
}

void Notification::setIconName(const QString &iconName)
{
    d->iconName = iconName;
}

NotificationAction *Notification::addAction(const QString &label)
{
    NotificationAction *action = new NotificationAction(label);
    action->setId(QString::number(d->actionIdCounter++));
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

Notification *Notification::exec(Notification::StandardEvent event,
                                 const QString &title,
                                 const QString &text)
{
    Notification *n = new Notification(event);
    n->setTitle(title);
    n->setText(text);
    QMetaObject::invokeMethod(n, &Notification::sendNotify, Qt::QueuedConnection);
    return n;
}

Notification *Notification::exec(const QString &title, const QString &text, const QString &iconName)
{
    Notification *n = new Notification();
    n->setTitle(title);
    n->setText(text);
    n->setIconName(iconName);
    QMetaObject::invokeMethod(n, &Notification::sendNotify, Qt::QueuedConnection);
    return n;
}

QList<NotificationAction *> Notification::actions() const
{
    return d->actions;
}

void Notification::activate(const QString &actionId)
{
    for (auto i = 0; i < d->actions.size(); ++i) {
        auto action = d->actions.at(i);
        if (action->id() == actionId) {
            Q_EMIT action->activated();
            break;
        }
    }
}

void Notification::ref()
{
    d->ref++;
}

void Notification::deRef()
{
    Q_ASSERT(d->ref > 0);
    --d->ref;
    if (d->ref == 0) {
        d->id = -1;
        close();
    }
}

void Notification::close()
{
    if (d->id >= 0) {
        NotificationManager::instance()->close(this);
    } else if (d->id == -1) {
        d->id = -2;
        emit closed();
        if (autoDestroy()) {
            deleteLater();
        }
    }
}

void Notification::sendNotify()
{
    NotificationManager::instance()->notify(this);
}

int Notification::id() const
{
    return d->id;
}
