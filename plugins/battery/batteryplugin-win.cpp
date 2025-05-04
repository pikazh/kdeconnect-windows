#include "batteryplugin-win.h"
#include "core/plugins/pluginfactory.h"
#include "plugin_battery_debug.h"

K_PLUGIN_CLASS_WITH_JSON(BatteryPlugin, "kdeconnect_battery.json")

int BatteryPlugin::g_instanceCount = 0;

void BatteryPlugin::receivePacket(const NetworkPacket &np)
{
    m_isRemoteCharging = np.get<bool>(QStringLiteral("isCharging"), false);
    m_remoteCharge = np.get<int>(QStringLiteral("currentCharge"), -1);
    //int thresholdEvent = np.get<int>(QStringLiteral("thresholdEvent"), (int) ThresholdNone);

    Q_EMIT refreshed(m_remoteCharge, m_isRemoteCharging);
}

BatteryMonitor *BatteryPlugin::batteryMonitor()
{
    static BatteryMonitor batteryMonitor;
    return &batteryMonitor;
}

void BatteryPlugin::localBatteryInfoUpdated()
{
    auto batteryMon = batteryMonitor();

    // Prepare an outgoing network packet
    NetworkPacket status(PACKET_TYPE_BATTERY, {{}});
    status.set(QStringLiteral("isCharging"), batteryMon->isCharging());
    status.set(QStringLiteral("currentCharge"), batteryMon->charge());
    // We consider the primary battery to be low if it's below 15%
    if (batteryMon->charge() <= m_chargeThreshold && !batteryMon->isCharging()) {
        status.set(QStringLiteral("thresholdEvent"), (int) ThresholdBatteryLow);
    } else {
        status.set(QStringLiteral("thresholdEvent"), (int) ThresholdNone);
    }
    sendPacket(status);
}

BatteryPlugin::BatteryPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
    
}

void BatteryPlugin::onPluginEnabled()
{
    auto batteryMon = batteryMonitor();
    QObject::connect(batteryMon,
                     &BatteryMonitor::refreshed,
                     this,
                     &BatteryPlugin::localBatteryInfoUpdated);

    if (++g_instanceCount == 1) {
        batteryMon->start();
    }
}

void BatteryPlugin::onPluginDisabled()
{
    auto batteryMon = batteryMonitor();
    QObject::disconnect(batteryMon, nullptr, this, nullptr);

    if (--g_instanceCount == 0) {
        batteryMon->stop();
    }
}

#include "batteryplugin-win.moc"
