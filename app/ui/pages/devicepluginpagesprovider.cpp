#include "devicepluginpagesprovider.h"
#include "mprisremotepage.h"
#include "remoteinputpage.h"

DevicePluginPagesProvider::DevicePluginPagesProvider(Device::Ptr device)
    : QObject(nullptr)
    , m_device(device)
{}

QList<BasePage *> DevicePluginPagesProvider::getPages()
{
    QList<BasePage *> pages;
    pages.append(new MprisRemotePage(m_device));
    pages.append(new RemoteInputPage(m_device));
    return pages;
}
