#pragma once

#include "core/plugins/kdeconnectplugin.h"

#include "batterymonitor.h"
#include "notification.h"

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

protected:
    virtual void onPluginEnabled() override;
    virtual void onPluginDisabled() override;

    virtual void receivePacket(const NetworkPacket &np) override;

    BatteryMonitor *batteryMonitor();

Q_SIGNALS:
    void refreshed(int charge, bool isCharging);

protected:
    void sendLocalBatteryInfo();

    void showNotification(const int chargePercent);

protected Q_SLOTS:
    void localBatteryInfoUpdated();
    void reloadConfig();

private:
    const int m_chargeThreshold = 15;
    int m_remoteCharge = -1;
    bool m_isRemoteCharging = false;

    bool m_showWarning = false;
    int m_warningThreshold = -1;
    bool m_showedWarning = false;

    static int g_instanceCount;
};
