#pragma once

#include <QWidget>

#include "core/device.h"
#include "ui/pages/BasePage.h"

#include "plugin/remotesystemvolumepluginwrapper.h"

namespace Ui {
class VolumeControlPage;
}

class VolumeDeviceItem;

class VolumeControlPage : public QWidget, public BasePage
{
    Q_OBJECT
public:
    explicit VolumeControlPage(Device::Ptr device, QWidget *parent = nullptr);
    virtual ~VolumeControlPage() override;

    virtual QString displayName() const override { return tr("Volume Control"); }
    virtual QIcon icon() const override;
    virtual QString id() const override { return QLatin1StringView("Volume Control"); }
    virtual bool apply() override;
    virtual QString helpPage() const override { return ""; }
    virtual bool shouldDisplay() const override;
    virtual void retranslate() override;

protected:
    enum Columns {
        Widget = 0,
        Name,
        Count,
    };

    VolumeDeviceItem *findVolumeDeviceItemByName(const QString &name);

protected Q_SLOTS:
    void initVolumeDeviceList();
    void onPluginVolumeChanged(const QString &name, int volume);
    void onPluginMuteChanged(const QString &name, bool mute);

private:
    Ui::VolumeControlPage *ui;
    RemoteSystemVolumePluginWrapper *m_remoteSystemVolumePluginWrapper = nullptr;
};
