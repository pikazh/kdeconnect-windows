#pragma once

#include "devicemanager.h"
#include "plugin/runcommandpluginwrapper.h"

#include <QHash>
#include <QString>

class RunCommandSetupListener : public QObject
{
    Q_OBJECT
public:
    explicit RunCommandSetupListener(DeviceManager *deviceManager, QObject *parent = nullptr);

protected Q_SLOTS:
    void registerSetupCallback(const QString &devId);
    void unRegisterSetupCallback(const QString &devId);

protected:
    void runCallback(const QString &devId);

private:
    DeviceManager *m_deviceManager;
    QHash<QString, RunCommandPluginWrapper *> m_runCommandPluginWrappers;
};
