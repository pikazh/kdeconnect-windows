#pragma once

#include <QDialog>

#include "core/device.h"
#include "core/plugins/kdeconnectplugin.h"
#include "core/plugins/kdeconnectpluginconfig.h"

namespace Ui {
class PresenterPluginConfigDialog;
}

class PresenterPluginConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PresenterPluginConfigDialog(Device::Ptr dev, QWidget *parent = nullptr);
    virtual ~PresenterPluginConfigDialog() override;

protected:
    void loadConfig();
    KdeConnectPluginConfig::Ptr pluginConfig();

protected Q_SLOTS:
    void saveConfig();

private:
    Device::Ptr m_device;
    Ui::PresenterPluginConfigDialog *ui;
};
