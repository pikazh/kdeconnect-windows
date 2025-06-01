#pragma once

#include <QDialog>
#include <QHash>
#include <QList>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QWidget>

#include "core/device.h"
#include "core/plugins/pluginid.h"

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

public Q_SLOTS:
    void showPluginConfigDialog(const PluginId &pluginID);
    void showPluginConfigDialog(const QString &pluginIDStr);

protected:
    void initPluginList();

protected Q_SLOTS:
    void on_filterEdit_textChanged(const QString &text);
    void filterAcceptPlugins();
    void saveEnabledPluginList();

private:
    Ui::PluginSettingsDialog *ui;

    Device::Ptr m_device;
    QList<QString> m_columnNames;
    QTimer *m_delayFilterTimer;
    QSet<QString> m_pluginIdsWhichHasConfig;
    QHash<QString, QWidget *> m_pluginConfigWidgets;
};
