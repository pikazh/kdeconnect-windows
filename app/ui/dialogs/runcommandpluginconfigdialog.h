#pragma once

#include <QDialog>
#include <QMenu>
#include <QString>

#include "core/device.h"
#include "core/plugins/kdeconnectplugin.h"
#include "core/plugins/kdeconnectpluginconfig.h"

namespace Ui {
class RunCommandPluginConfigDialog;
}

class RunCommandPluginConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RunCommandPluginConfigDialog(Device::Ptr dev, QWidget *parent = nullptr);
    virtual ~RunCommandPluginConfigDialog() override;

protected:
    enum Columns { Name = 0, Command, DeleteButton };

    void loadConfig();
    KdeConnectPluginConfig::Ptr pluginConfig();

    void addSuggestedCommand(QMenu *menu, const QString &name, const QString &command);
    void addCommandToTable(const QString &name, const QString &command, const QString &key);
    void deletAllRows();

protected Q_SLOTS:

    void saveConfig();

    void on_importButton_clicked();
    void on_exportButton_clicked();

private:
    Device::Ptr m_device;
    Ui::RunCommandPluginConfigDialog *ui;
};
