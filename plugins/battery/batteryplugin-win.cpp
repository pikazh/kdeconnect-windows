#include "batteryplugin-win.h"
#include "core/plugins/pluginfactory.h"
#include "plugin_battery_debug.h"

#include <Windows.h>

K_PLUGIN_CLASS_WITH_JSON(BatteryPlugin, "kdeconnect_battery.json")

void BatteryPlugin::receivePacket(const NetworkPacket &np)
{
    m_isRemoteCharging = np.get<bool>(QStringLiteral("isCharging"), false);
    m_remoteCharge = np.get<int>(QStringLiteral("currentCharge"), -1);
    int thresholdEvent = np.get<int>(QStringLiteral("thresholdEvent"), (int) ThresholdNone);

    Q_EMIT refreshed(m_isRemoteCharging, m_remoteCharge);
}

void BatteryPlugin::checkAndNotifyChargeInfo()
{
    SYSTEM_POWER_STATUS sps = {0};
    if (FALSE != GetSystemPowerStatus(&sps)) {
        bool isCharging = sps.ACLineStatus == 1;
        int charge = sps.BatteryLifePercent;

        if (m_isLocalCharging != isCharging || m_localCharge != charge) {
            m_isLocalCharging = isCharging;
            m_localCharge = charge;

            // Prepare an outgoing network packet
            NetworkPacket status(PACKET_TYPE_BATTERY, {{}});
            status.set(QStringLiteral("isCharging"), m_isLocalCharging);
            status.set(QStringLiteral("currentCharge"), m_localCharge);
            // We consider the primary battery to be low if it's below 15%
            if (m_localCharge <= m_chargeThreshold && !m_isLocalCharging) {
                status.set(QStringLiteral("thresholdEvent"), (int) ThresholdBatteryLow);
            } else {
                status.set(QStringLiteral("thresholdEvent"), (int) ThresholdNone);
            }
            sendPacket(status);
        }
    } else {
        qFatal(KDECONNECT_PLUGIN_BATTERY) << "failed to get system power status";
    }
}

BatteryPlugin::BatteryPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
    , m_checkChargeInfoTimer(new QTimer(this))
{
    QObject::connect(m_checkChargeInfoTimer,
                     &QTimer::timeout,
                     this,
                     &BatteryPlugin::checkAndNotifyChargeInfo);
}

void BatteryPlugin::onPluginEnabled()
{
    QMetaObject::invokeMethod(this, "checkAndNotifyChargeInfo", Qt::QueuedConnection);

    // check power status every minute
    m_checkChargeInfoTimer->setInterval(1000 * 60);
    m_checkChargeInfoTimer->start();
}

#include "batteryplugin-win.moc"
