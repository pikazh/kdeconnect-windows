#include "pingplugin.h"
#include "core/plugins/pluginfactory.h"
#include "notification.h"
#include "plugin_ping_debug.h"

K_PLUGIN_CLASS_WITH_JSON(PingPlugin, "kdeconnect_ping.json")

void PingPlugin::receivePacket(const NetworkPacket &np)
{
    Notification::exec(device()->name(),
                       np.get<QString>(QStringLiteral("message"), tr("Ping!")),
                       QStringLiteral("dialog-ok-apply"));
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
