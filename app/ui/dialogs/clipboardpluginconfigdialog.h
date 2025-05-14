#pragma once

#include <QDialog>

#include "core/device.h"
#include "core/plugins/kdeconnectplugin.h"
#include "core/plugins/kdeconnectpluginconfig.h"

namespace Ui {
class ClipboardPluginConfigDialog;
}

class ClipboardPluginConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClipboardPluginConfigDialog(Device::Ptr dev, QWidget *parent = nullptr);
    virtual ~ClipboardPluginConfigDialog() override;

protected:
    void loadConfig();
    KdeConnectPluginConfig::Ptr pluginConfig();

protected Q_SLOTS:
    void saveConfig();

private:
    Device::Ptr m_device;
    Ui::ClipboardPluginConfigDialog *ui;
};
