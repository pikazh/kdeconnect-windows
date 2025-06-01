#include "pluginsettingsdialog.h"
#include "ui_pluginsettingsdialog.h"

#include "core/plugins/kdeconnectplugin.h"
#include "core/plugins/pluginid.h"
#include "core/plugins/pluginloader.h"

#include "ui/dialogs/batterypluginconfigdialog.h"
#include "ui/dialogs/clipboardpluginconfigdialog.h"
#include "ui/dialogs/runcommandpluginconfigdialog.h"

#include <QEvent>
#include <QIcon>
#include <QPushButton>
#include <QTreeWidgetItem>

PluginSettingsDialog::PluginSettingsDialog(Device::Ptr device, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PluginSettingsDialog)
    , m_device(device)
    , m_columnNames{{tr("Enable")}, {tr("Name")}, {tr("Description")}, {tr("Configuration")}}
    , m_delayFilterTimer(new QTimer(this))
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    QString title = tr("Plugins Configuration - ") + m_device->name();
    setWindowTitle(title);

    m_pluginIdsWhichHasConfig.insert(pluginIdString(PluginId::BatteryMonitor));
    m_pluginIdsWhichHasConfig.insert(pluginIdString(PluginId::ClipBoard));
    m_pluginIdsWhichHasConfig.insert(pluginIdString(PluginId::RunCommand));

    QObject::connect(this,
                     &PluginSettingsDialog::accepted,
                     this,
                     &PluginSettingsDialog::saveEnabledPluginList);

    m_delayFilterTimer->setInterval(200);
    m_delayFilterTimer->setSingleShot(true);
    QObject::connect(m_delayFilterTimer,
                     &QTimer::timeout,
                     this,
                     &PluginSettingsDialog::filterAcceptPlugins);

    ui->pluginList->setColumnCount(m_columnNames.size());
    ui->pluginList->setHeaderLabels(m_columnNames);
    ui->pluginList->sortItems(Name, Qt::AscendingOrder);

    initPluginList();
}

PluginSettingsDialog::~PluginSettingsDialog()
{
    delete ui;
}

void PluginSettingsDialog::showPluginConfigDialog(const PluginId &pluginID)
{
    showPluginConfigDialog(pluginIdString(pluginID));
}

void PluginSettingsDialog::initPluginList()
{
    ui->pluginList->clear();
    auto pluginLoader = PluginLoader::instance();
    auto pluginList = pluginLoader->getPluginList();
    for (auto it = pluginList.begin(); it != pluginList.end(); ++it) {
        QString &pluginId = (*it);
        auto info = pluginLoader->getPluginInfo(pluginId);
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setCheckState(Enabled,
                            m_device->isPluginEnabled(pluginId) ? Qt::Checked : Qt::Unchecked);
        item->setData(Enabled, Qt::UserRole, pluginId);
        item->setText(Name, info.name());
        item->setIcon(Name, QIcon::fromTheme(info.iconName()));
        item->setText(Description, info.description());

        ui->pluginList->addTopLevelItem(item);

        if (m_pluginIdsWhichHasConfig.contains(pluginId)) {
            QWidget *w = new QWidget(ui->pluginList);
            QHBoxLayout *layout = new QHBoxLayout(w);
            layout->setContentsMargins(0, 0, 0, 0);
            QPushButton *configBtn = new QPushButton(w);
            configBtn->setProperty("pluginId", pluginId);
            configBtn->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
            layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
            layout->addWidget(configBtn);
            layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

            ui->pluginList->setItemWidget(item, Configuration, w);

            QObject::connect(configBtn, &QPushButton::clicked, this, [this]() {
                auto btn = QObject::sender();
                if (btn != nullptr) {
                    QString pluginId = qvariant_cast<QString>(btn->property("pluginId"));
                    showPluginConfigDialog(pluginId);
                }
            });
        }
    }

    ui->pluginList->resizeColumnToContents(Enabled);
    ui->pluginList->resizeColumnToContents(Name);
    ui->pluginList->resizeColumnToContents(Description);
    ui->pluginList->resizeColumnToContents(Configuration);
}

void PluginSettingsDialog::filterAcceptPlugins()
{
    QString filterText = ui->filterEdit->text().trimmed();
    for (auto i = 0; i < ui->pluginList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui->pluginList->topLevelItem(i);
        if (filterText.isEmpty()
            || item->text(Name).indexOf(filterText, 0, Qt::CaseInsensitive) > -1
            || item->text(Description).indexOf(filterText, 0, Qt::CaseInsensitive) > -1) {
            item->setHidden(false);
        } else {
            item->setHidden(true);
        }
    }
}

void PluginSettingsDialog::saveEnabledPluginList()
{
    for (auto i = 0; i < ui->pluginList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui->pluginList->topLevelItem(i);
        QString pluginId = qvariant_cast<QString>(item->data(Enabled, Qt::UserRole));
        if (!pluginId.isEmpty())
            m_device->setPluginEnabled(pluginId, item->checkState(Enabled) == Qt::Checked);
    }

    m_device->reloadPlugins();
}

void PluginSettingsDialog::showPluginConfigDialog(const QString &pluginIDStr)
{
    if (!m_pluginIdsWhichHasConfig.contains(pluginIDStr)) {
        return;
    }

    auto it = m_pluginConfigWidgets.find(pluginIDStr);
    if (it != m_pluginConfigWidgets.end()) {
        it.value()->showNormal();
        it.value()->raise();
        it.value()->activateWindow();
    } else {
        auto pluginLoader = PluginLoader::instance();
        auto pluginInfo = pluginLoader->getPluginInfo(pluginIDStr);

        QDialog *dlg = nullptr;
        if (pluginIDStr == pluginIdString(PluginId::BatteryMonitor)) {
            dlg = new BatteryPluginConfigDialog(m_device, this);
            dlg->setWindowTitle(pluginInfo.name() + QStringLiteral(" - ") + m_device->name());

        } else if (pluginIDStr == pluginIdString(PluginId::ClipBoard)) {
            dlg = new ClipboardPluginConfigDialog(m_device, this);
            dlg->setWindowTitle(pluginInfo.name() + QStringLiteral(" - ") + m_device->name());

        } else if (pluginIDStr == pluginIdString(PluginId::RunCommand)) {
            dlg = new RunCommandPluginConfigDialog(m_device, this);
            dlg->setWindowTitle(pluginInfo.name() + QStringLiteral(" - ") + m_device->name());
        }

        if (dlg != nullptr) {
            m_pluginConfigWidgets[pluginIDStr] = dlg;
            QObject::connect(dlg, &QDialog::finished, this, [this, pluginIDStr]() {
                m_pluginConfigWidgets.remove(pluginIDStr);
            });
            dlg->showNormal();
            dlg->raise();
            dlg->activateWindow();
        }
    }
}

void PluginSettingsDialog::on_filterEdit_textChanged(const QString &text)
{
    m_delayFilterTimer->start();
}
