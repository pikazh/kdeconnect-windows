#include "devicepluginpagesprovider.h"
#include "filetransferpage.h"
#include "mprisremotepage.h"
#include "remotecommandspage.h"
#include "remoteinputpage.h"
#include "volumecontrolpage.h"

DevicePluginPagesProvider::DevicePluginPagesProvider(Device::Ptr device)
    : QObject(nullptr)
    , m_device(device)
{}

QList<BasePage *> DevicePluginPagesProvider::getPages()
{
    QList<BasePage *> pages;
    pages.append(new RemoteCommandsPage(m_device));
    pages.append(new MprisRemotePage(m_device));
    pages.append(new VolumeControlPage(m_device));
    pages.append(new RemoteInputPage(m_device));
    pages.append(new FileTransferPage(m_device));
    return pages;
}
