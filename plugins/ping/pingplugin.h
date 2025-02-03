#pragma once

#include "core/kdeconnectplugin.h"
#include <QObject>

#define PACKET_TYPE_PING QStringLiteral("kdeconnect.ping")

class PingPlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.ping")

public:
    PingPlugin(QObject *parent, const QVariantList &args);

    Q_SCRIPTABLE void sendPing();
    Q_SCRIPTABLE void sendPing(const QString &customMessage);

    void receivePacket(const NetworkPacket &np) override;
    QString dbusPath() const override;
};

