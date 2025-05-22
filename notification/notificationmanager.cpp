#include "notificationmanager.h"
#include "notifybysnore.h"

#include <QHash>
#include <QString>

struct NotificationManagerPrivate
{
    QHash<int, Notification *> notifications;
    QHash<int, int> notificationsRefCount;
    QHash<QString, NotificationPlugin *> notificationPlugins;
};

NotificationManager *NotificationManager::instance()
{
    static NotificationManager self;
    return &self;
}

bool NotificationManager::notify(Notification *n)
{
    auto pluginList = pluginsForNotification(n);
    if (pluginList.isEmpty()) {
        // todo
        return false;
    }

    d->notifications[n->id()] = n;

    for (auto plugin : pluginList) {
        notificationAddRef(n->id());
        plugin->notify(n);
    }

    return true;
}

void NotificationManager::close(int id)
{
    auto it = d->notifications.find(id);
    if (it != d->notifications.end()) {
        auto notif = it.value();
        auto pluginList = pluginsForNotification(notif);
        for (auto plugin : pluginList) {
            plugin->close(notif->id());
        }
    }
}

NotificationManager::NotificationManager(QObject *parent)
    : QObject{parent}
    , d(new NotificationManagerPrivate)
{}

NotificationManager::~NotificationManager()
{
    qDeleteAll(d->notificationPlugins);
    d->notificationPlugins.clear();
}

QList<NotificationPlugin *> NotificationManager::pluginsForNotification(Notification *n)
{
    // just return NotifyBySnore object now
    auto it = d->notificationPlugins.find(QStringLiteral("default"));
    if (it != d->notificationPlugins.end()) {
        return QList<NotificationPlugin *>({it.value()});
    } else {
        NotifyBySnore *plugin = new NotifyBySnore(this);
        QObject::connect(plugin,
                         &NotifyBySnore::finished,
                         this,
                         &NotificationManager::onPluginNotifyFinished);
        QObject::connect(plugin,
                         &NotifyBySnore::actionInvoked,
                         this,
                         &NotificationManager::onPluginNotificationActionInvoked);

        plugin->init();
        d->notificationPlugins[QStringLiteral("default")] = plugin;

        return QList<NotificationPlugin *>({plugin});
    }
}

int NotificationManager::notificationAddRef(int id)
{
    auto it = d->notificationsRefCount.find(id);
    if (it != d->notificationsRefCount.end()) {
        return ++it.value();
    } else {
        d->notificationsRefCount[id] = 1;
        return 1;
    }
}

int NotificationManager::notificationDeRef(int id)
{
    auto it = d->notificationsRefCount.find(id);
    if (it != d->notificationsRefCount.end()) {
        int ret = --it.value();
        if (ret == 0) {
            d->notificationsRefCount.erase(it);
        }

        return ret;
    } else {
        Q_ASSERT(0);
        return -1;
    }
}

void NotificationManager::onPluginNotifyFinished(int id)
{
    auto it = d->notifications.find(id);
    if (it != d->notifications.end()) {
        if (notificationDeRef(id) == 0) {
            auto n = it.value();
            d->notifications.erase(it);

            n->emitClosed();
        }
    }
}

void NotificationManager::onPluginNotificationActionInvoked(int id, const QString &actionId)
{
    auto it = d->notifications.find(id);
    if (it != d->notifications.end()) {
        it.value()->activate(actionId);
    }
}
