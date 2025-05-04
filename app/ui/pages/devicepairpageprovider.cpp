#include "devicepairpageprovider.h"
#include "pairpage.h"

DevicePairPageProvider::DevicePairPageProvider(Device::Ptr device)
    : QObject(nullptr)
    , m_device(device)
{}

QList<BasePage *> DevicePairPageProvider::getPages()
{
    QList<BasePage *> pages;
    pages.append(new PairPage(m_device));
    return pages;
}
