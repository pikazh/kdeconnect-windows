#include "batteryplugin-win.h"
#include "core/plugins/pluginfactory.h"
#include "notification.h"
#include "plugin_battery_debug.h"

K_PLUGIN_CLASS_WITH_JSON(BatteryPlugin, "kdeconnect_battery.json")

int BatteryPlugin::g_instanceCount = 0;

void BatteryPlugin::receivePacket(const NetworkPacket &np)
{
    m_isRemoteCharging = np.get<bool>(QStringLiteral("isCharging"), false);
    m_remoteCharge = np.get<int>(QStringLiteral("currentCharge"), -1);
    //int thresholdEvent = np.get<int>(QStringLiteral("thresholdEvent"), (int) ThresholdNone);

    Q_EMIT refreshed(m_remoteCharge, m_isRemoteCharging);

    if (!m_isRemoteCharging && m_showWarning && m_remoteCharge <= m_warningThreshold) {
        if (!m_showedWarning) {
            m_showedWarning = true;
            showNotification(m_remoteCharge);
        }
    } else {
        m_showedWarning = false;
    }
}

BatteryMonitor *BatteryPlugin::batteryMonitor()
{
    static BatteryMonitor batteryMonitor;
    return &batteryMonitor;
}

void BatteryPlugin::sendLocalBatteryInfo()
{
    auto batteryMon = batteryMonitor();

    // Prepare an outgoing network packet
    NetworkPacket status(PACKET_TYPE_BATTERY);
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

void BatteryPlugin::showNotification(const int chargePercent)
{
    QString title = QString(tr("%1: Low Battery")).arg(device()->name());
    QString text = QString(tr("Battery at %1%")).arg(chargePercent);
    QString iconName;
    if (chargePercent <= 10) {
        iconName = QStringLiteral("battery-010");
    } else if (chargePercent <= 20) {
        iconName = QStringLiteral("battery-020");
    } else if (chargePercent <= 30) {
        iconName = QStringLiteral("battery-030");
    } else if (chargePercent <= 40) {
        iconName = QStringLiteral("battery-040");
    } else {
        iconName = QStringLiteral("battery-050");
    }

    Notification::exec(title, text, iconName);
}

void BatteryPlugin::localBatteryInfoUpdated()
{
    sendLocalBatteryInfo();
}

void BatteryPlugin::reloadConfig()
{
    auto conf = config();
    m_showWarning = conf->getBool(QStringLiteral("warning"), true);
    m_warningThreshold = conf->getInt(QStringLiteral("threshold"), m_chargeThreshold);
}

BatteryPlugin::BatteryPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
    auto conf = config();
    QObject::connect(conf.get(),
                     &KdeConnectPluginConfig::configChanged,
                     this,
                     &BatteryPlugin::reloadConfig);

    reloadConfig();

    auto batteryMon = batteryMonitor();
    QObject::connect(batteryMon,
                     &BatteryMonitor::refreshed,
                     this,
                     &BatteryPlugin::localBatteryInfoUpdated);
}

void BatteryPlugin::onPluginEnabled()
{
    auto batteryMon = batteryMonitor();
    if (++g_instanceCount == 1) {
        batteryMon->start();
    }

    sendLocalBatteryInfo();
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
