#include "sharepluginwrapper.h"

SharePluginWrapper::SharePluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::Share, parent)
{}

QSharedPointer<TaskScheduler> SharePluginWrapper::recvFilesTaskSchedule()
{
    return propertyValue<QSharedPointer<TaskScheduler>>("recvFilesTaskSchedule");
}

QSharedPointer<TaskScheduler> SharePluginWrapper::sendFilesTaskSchedule()
{
    return propertyValue<QSharedPointer<TaskScheduler>>("sendFilesTaskSchedule");
}

void SharePluginWrapper::shareText(const QString &text)
{
    invokeMethod("shareText", text);
}

void SharePluginWrapper::shareUrl(const QUrl &url)
{
    invokeMethod("shareUrl", url);
}
