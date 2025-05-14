#pragma once

#include <QObject>

#include "core/plugins/kdeconnectplugin.h"

#define PACKET_TYPE_PING QStringLiteral("kdeconnect.ping")

class PingPlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    PingPlugin(QObject *parent, const QVariantList &args);

public Q_SLOTS:
    void sendPing();
    void sendPing(const QString &customMessage);

protected:
    virtual void receivePacket(const NetworkPacket &np) override;
};

