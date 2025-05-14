#pragma once

#include <QDialog>
#include <QList>
#include <QSet>
#include <QString>
#include <QTimer>

#include "core/device.h"

namespace Ui {
class PluginSettingsDialog;
}

class PluginSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    enum Columns { Enabled = 0, Name, Description, Configuration };
    explicit PluginSettingsDialog(Device::Ptr device, QWidget *parent = nullptr);
    virtual ~PluginSettingsDialog() override;

protected:
    void initPluginList();

protected Q_SLOTS:
    void on_filterEdit_textChanged(const QString &text);
    void filterAcceptPlugins();
    void saveEnabledPluginList();
    void showPluginConfigDialog();

private:
    Ui::PluginSettingsDialog *ui;

    Device::Ptr m_device;
    QList<QString> m_columnNames;
    QTimer *m_delayFilterTimer;
    QSet<QString> m_pluginIdsWhichHasConfig;
};
