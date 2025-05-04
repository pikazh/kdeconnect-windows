#pragma once

#include <QObject>
#include <QTimer>

#include "core/plugins/kdeconnectplugin.h"

#define PACKET_TYPE_BATTERY QStringLiteral("kdeconnect.battery")

class BatteryPlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_PROPERTY(int charge READ charge NOTIFY refreshed)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY refreshed)
public:
    // Keep these values in sync with THRESHOLD* constants in
    // kdeconnect-android:BatteryPlugin.java
    // see README for their meaning
    enum ThresholdBatteryEvent {
        ThresholdNone = 0,
        ThresholdBatteryLow = 1,
    };
    BatteryPlugin(QObject *parent, const QVariantList &args);

    int charge() const { return m_remoteCharge; }
    bool isCharging() const { return m_isRemoteCharging; }

    virtual void onPluginEnabled() override;

protected:
    virtual void receivePacket(const NetworkPacket &np) override;

Q_SIGNALS:
    void refreshed(bool isCharging, int charge);

protected Q_SLOTS:
    void checkAndNotifyChargeInfo();

private:
    int m_localCharge = -1;
    bool m_isLocalCharging = false;

    int m_remoteCharge = -1;
    bool m_isRemoteCharging = false;

    const int m_chargeThreshold = 15;
    QTimer *m_checkChargeInfoTimer;
};
