#include "batterymonitor.h"
#include "plugin_battery_debug.h"

#include <Windows.h>

BatteryMonitor::BatteryMonitor(QObject *parent)
    : QObject{parent}
    , m_checkBatteryInfoTimer(new QTimer(this))
{
    QObject::connect(m_checkBatteryInfoTimer, &QTimer::timeout, this, [this]() {
        updateSysBatteryInfo();
    });

    m_checkBatteryInfoTimer->setInterval(60 * 1000);
}

void BatteryMonitor::start()
{
    updateSysBatteryInfo();
    m_checkBatteryInfoTimer->start();
}

void BatteryMonitor::stop()
{
    m_checkBatteryInfoTimer->stop();
}

void BatteryMonitor::updateSysBatteryInfo()
{
    SYSTEM_POWER_STATUS sps = {0};
    if (FALSE != GetSystemPowerStatus(&sps)) {
        bool isCharging = sps.ACLineStatus == 1;
        int charge = sps.BatteryLifePercent;

        if (m_isLocalCharging != isCharging || m_localCharge != charge) {
            m_isLocalCharging = isCharging;
            m_localCharge = charge;
            emit refreshed(m_localCharge, m_isLocalCharging);
        }
    } else {
        qCritical(KDECONNECT_PLUGIN_BATTERY) << "failed to get system power status";
    }
}
