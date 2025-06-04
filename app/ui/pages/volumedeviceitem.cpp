#include "volumedeviceitem.h"
#include "ui_volumedeviceitem.h"

#include "app_debug.h"

#include <QIcon>

VolumeDeviceItem::VolumeDeviceItem(QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::VolumeDeviceItem)
    , m_delayChangeTimer(new QTimer(this))
{
    m_delayChangeTimer->setSingleShot(true);
    m_delayChangeTimer->setInterval(200);
    QObject::connect(m_delayChangeTimer, &QTimer::timeout, this, [this]() {
        int val = ui->volumeSlider->value();
        Q_EMIT volumeChanged(val);
    });
    ui->setupUi(this);
    ui->volumeSlider->setTracking(false);
    ui->muteButton->setIcon(QIcon::fromTheme(QStringLiteral("audio-volume-high")));
    ui->muteButton->setToolTip(tr("Mute"));
}

VolumeDeviceItem::~VolumeDeviceItem()
{
    delete ui;
}

void VolumeDeviceItem::setVolumeDeviceName(const QString &name, const QString &desc)
{
    ui->volumeDeviceName->setText(name);
    ui->volumeDeviceName->setToolTip(desc);
}

void VolumeDeviceItem::setVolume(int volume)
{
    ui->volumeSlider->setSliderPosition(volume);
}

void VolumeDeviceItem::setVolumeRange(int minVal, int maxVal)
{
    ui->volumeSlider->setRange(minVal, maxVal);
}

void VolumeDeviceItem::setMute(bool mute, bool triggerSignal)
{
    m_muted = mute;
    ui->muteButton->setIcon(QIcon::fromTheme(m_muted ? QStringLiteral("audio-volume-muted")
                                                     : QStringLiteral("audio-volume-high")));
    ui->muteButton->setToolTip(mute ? tr("Unmute") : tr("Mute"));
    ui->volumeSlider->setEnabled(!m_muted);

    if (triggerSignal)
        Q_EMIT muteChanged(m_muted);
}

void VolumeDeviceItem::retranslateUi()
{
    ui->retranslateUi(this);
}

void VolumeDeviceItem::on_muteButton_clicked()
{
    setMute(!m_muted, true);
}

void VolumeDeviceItem::on_volumeSlider_valueChanged(int val)
{
    m_delayChangeTimer->start();
}
