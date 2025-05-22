#include "notification.h"
#include "notification_debug.h"
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
    QString title;
    QString text;
    QPixmap pixmap;
    QString iconName;
    QList<NotificationAction*> actions;
    bool autoDestroy = true;
    bool isClosed = true;

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
    qDebug(NOTIFICATIONS_MOD) << "notification with id" << d->id << "created.";
}

Notification::Notification(StandardEvent eventId, QObject *parent)
    : QObject(parent)
    , d(new NotificationPrivate)
{
    d->id = d->notificationIdCounter++;
    setIconName(standardEventToIconName(eventId));

    qDebug(NOTIFICATIONS_MOD) << "notification with id" << d->id << "created.";
}

Notification::~Notification()
{
    clearActions();
    if (!isClosed()) {
        close();
    }

    qDebug(NOTIFICATIONS_MOD) << "notification with id" << d->id << "destroyed.";
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

bool Notification::autoDestroy() const
{
    return d->autoDestroy;
}

void Notification::setAutoDestroy(bool autoDestroy)
{
    d->autoDestroy = autoDestroy;
}

bool Notification::isClosed() const
{
    return d->isClosed;
}

Notification *Notification::exec(Notification::StandardEvent event,
                                 const QString &title,
                                 const QString &text)
{
    auto n(new Notification(event));
    n->setTitle(title);
    n->setText(text);
    QMetaObject::invokeMethod(n, &Notification::sendNotify, Qt::QueuedConnection);
    return n;
}

Notification *Notification::exec(const QString &title, const QString &text, const QString &iconName)
{
    auto n(new Notification());
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

void Notification::close()
{
    NotificationManager::instance()->close(this->id());
}

void Notification::sendNotify()
{
    if (NotificationManager::instance()->notify(this)) {
        d->isClosed = false;
    }
}

void Notification::emitClosed()
{
    d->isClosed = true;
    Q_EMIT closed();
    qDebug(NOTIFICATIONS_MOD) << "notification with id" << d->id << "closed.";
    if (autoDestroy()) {
        deleteLater();
    }
}

int Notification::id() const
{
    return d->id;
}
