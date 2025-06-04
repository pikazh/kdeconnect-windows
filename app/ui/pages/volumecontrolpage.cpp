#include "volumecontrolpage.h"
#include "ui_volumecontrolpage.h"

#include "volumedeviceitem.h"

#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTreeWidgetItem>

VolumeControlPage::VolumeControlPage(Device::Ptr device, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VolumeControlPage)
    , m_remoteSystemVolumePluginWrapper(new RemoteSystemVolumePluginWrapper(device, this))
{
    ui->setupUi(this);
    ui->volumeDeviceList->setColumnCount(Columns::Count);
    ui->volumeDeviceList->setColumnHidden(Columns::Name, true);
    ui->volumeDeviceList->sortByColumn(Columns::Name, Qt::AscendingOrder);

    QObject::connect(m_remoteSystemVolumePluginWrapper,
                     &RemoteSystemVolumePluginWrapper::volumeChanged,
                     this,
                     &VolumeControlPage::onPluginVolumeChanged);
    QObject::connect(m_remoteSystemVolumePluginWrapper,
                     &RemoteSystemVolumePluginWrapper::mutedChanged,
                     this,
                     &VolumeControlPage::onPluginMuteChanged);
    QObject::connect(m_remoteSystemVolumePluginWrapper,
                     &RemoteSystemVolumePluginWrapper::sinksChanged,
                     this,
                     &VolumeControlPage::initVolumeDeviceList);

    m_remoteSystemVolumePluginWrapper->init();

    initVolumeDeviceList();
}

VolumeControlPage::~VolumeControlPage()
{
    delete ui;
}

QIcon VolumeControlPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("player-volume"));
}

bool VolumeControlPage::apply()
{
    return true;
}

bool VolumeControlPage::shouldDisplay() const
{
    return m_remoteSystemVolumePluginWrapper->isPluginLoaded();
}

void VolumeControlPage::retranslate()
{
    ui->retranslateUi(this);
    auto itemCount = ui->volumeDeviceList->topLevelItemCount();
    for (auto i = 0; i < itemCount; ++i) {
        auto item = ui->volumeDeviceList->topLevelItem(i);
        VolumeDeviceItem *devItem = qobject_cast<VolumeDeviceItem *>(
            ui->volumeDeviceList->itemWidget(item, Columns::Widget));
        if (devItem != nullptr)
            devItem->retranslateUi();
    }
}

void VolumeControlPage::initVolumeDeviceList()
{
    ui->volumeDeviceList->clear();
    auto sinsData = m_remoteSystemVolumePluginWrapper->sinks();
    const auto cmds = QJsonDocument::fromJson(sinsData).array();
    for (const QJsonValue &cmd : cmds) {
        const QJsonObject cont = cmd.toObject();

        QString name = cont.value(QStringLiteral("name")).toString();
        QString description = cont.value(QStringLiteral("description")).toString();
        int maxVolume = cont.value(QStringLiteral("maxVolume")).toInt();
        int volume = cont.value(QStringLiteral("volume")).toInt();
        bool muted = cont.value(QStringLiteral("muted")).toBool();
        bool enabled = cont.value(QStringLiteral("enabled")).toBool();

        auto item = new QTreeWidgetItem();
        item->setText(Columns::Name, name);
        ui->volumeDeviceList->addTopLevelItem(item);

        auto volumeDevItem = new VolumeDeviceItem(ui->volumeDeviceList);
        volumeDevItem->setVolumeDeviceName(name, description);
        volumeDevItem->setVolumeRange(0, maxVolume);
        volumeDevItem->setVolume(volume);
        volumeDevItem->setMute(muted);
        volumeDevItem->setEnabled(enabled);
        if (!enabled) {
            volumeDevItem->setToolTip(tr("Device disabled"));
        }
        QObject::connect(volumeDevItem,
                         &VolumeDeviceItem::volumeChanged,
                         this,
                         [this, name](int val) {
                             m_remoteSystemVolumePluginWrapper->sendVolume(name, val);
                         });
        QObject::connect(volumeDevItem,
                         &VolumeDeviceItem::muteChanged,
                         this,
                         [this, name](bool mute) {
                             m_remoteSystemVolumePluginWrapper->sendMuted(name, mute);
                         });

        ui->volumeDeviceList->setItemWidget(item, Columns::Widget, volumeDevItem);
    }
}

VolumeDeviceItem *VolumeControlPage::findVolumeDeviceItemByName(const QString &name)
{
    auto itemCount = ui->volumeDeviceList->topLevelItemCount();
    for (auto i = 0; i < itemCount; ++i) {
        auto item = ui->volumeDeviceList->topLevelItem(i);
        QString devName = item->text(Columns::Name);
        if (devName.compare(name, Qt::CaseInsensitive) == 0) {
            VolumeDeviceItem *devItem = qobject_cast<VolumeDeviceItem *>(
                ui->volumeDeviceList->itemWidget(item, Columns::Widget));
            return devItem;
        }
    }
    Q_ASSERT(0);

    return nullptr;
}

void VolumeControlPage::onPluginVolumeChanged(const QString &name, int val)
{
    auto devItem = findVolumeDeviceItemByName(name);
    Q_ASSERT(devItem != nullptr);
    if (devItem != nullptr) {
        devItem->setVolume(val);
    }
}

void VolumeControlPage::onPluginMuteChanged(const QString &name, bool mute)
{
    auto devItem = findVolumeDeviceItemByName(name);
    Q_ASSERT(devItem != nullptr);
    if (devItem != nullptr) {
        devItem->setMute(mute);
    }
}
