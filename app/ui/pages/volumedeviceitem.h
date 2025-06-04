#pragma once

#include <QTimer>
#include <QWidget>

namespace Ui {
class VolumeDeviceItem;
}

class VolumeDeviceItem : public QWidget
{
    Q_OBJECT
public:
    explicit VolumeDeviceItem(QWidget *parent = nullptr);
    virtual ~VolumeDeviceItem() override;

    void setVolumeDeviceName(const QString &name, const QString &desc);
    void setVolume(int volume);
    void setVolumeRange(int minVal, int maxVal);
    void setMute(bool mute, bool triggerSignal = false);
    void retranslateUi();

protected Q_SLOTS:
    void on_muteButton_clicked();
    void on_volumeSlider_valueChanged(int val);

Q_SIGNALS:
    void volumeChanged(int v);
    void muteChanged(bool m);

private:
    Ui::VolumeDeviceItem *ui;
    QTimer *m_delayChangeTimer = nullptr;
    bool m_muted = false;
};
