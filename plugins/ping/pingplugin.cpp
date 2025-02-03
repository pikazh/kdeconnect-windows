#include "pingplugin.h"
#include "plugin_ping_debug.h"
#include "pluginfactory.h"
#include "core/daemon.h"

K_PLUGIN_CLASS_WITH_JSON(PingPlugin, "kdeconnect_ping.json")

void PingPlugin::receivePacket(const NetworkPacket &np)
{
    Daemon::instance()->sendSimpleNotification(QStringLiteral("pingReceived"),
                                               device()->name(),
                                               np.get<QString>(QStringLiteral("message"), tr("Ping!")),
                                               QStringLiteral("dialog-ok"));
}

QString PingPlugin::dbusPath() const
{
    return QStringLiteral("/modules/kdeconnect/devices/%1/ping").arg(device()->id());
}

PingPlugin::PingPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
}

void PingPlugin::sendPing()
{
    NetworkPacket np(PACKET_TYPE_PING);
    bool success = sendPacket(np);
    qCDebug(KDECONNECT_PLUGIN_PING) << "sendPing:" << success;
}

void PingPlugin::sendPing(const QString &customMessage)
{
    NetworkPacket np(PACKET_TYPE_PING);
    if (!customMessage.isEmpty()) {
        np.set(QStringLiteral("message"), customMessage);
    }
    bool success = sendPacket(np);
    qCDebug(KDECONNECT_PLUGIN_PING) << "sendPing:" << success;
}

#include "pingplugin.moc"
