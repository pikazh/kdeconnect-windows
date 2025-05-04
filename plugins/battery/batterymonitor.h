#pragma once

#include <QObject>
#include <QTimer>

class BatteryMonitor : public QObject
{
    Q_OBJECT
public:
    explicit BatteryMonitor(QObject *parent = nullptr);
    virtual ~BatteryMonitor() override = default;

    int charge() const { return m_localCharge; }
    bool isCharging() const { return m_isLocalCharging; }
    void start();
    void stop();

protected:
    void updateSysBatteryInfo();

signals:
    void refreshed(int charge, bool isCharging);

private:
    int m_localCharge = -1;
    bool m_isLocalCharging = false;
    QTimer *m_checkBatteryInfoTimer = nullptr;
};
