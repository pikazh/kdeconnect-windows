#include "pluginsettingsdialog.h"
#include "core/plugins/kdeconnectplugin.h"
#include "core/plugins/pluginloader.h"
#include "ui_pluginsettingsdialog.h"

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

    ui->setupUi(this);

    ui->pluginList->setColumnCount(m_columnNames.size());
    ui->pluginList->setHeaderLabels(m_columnNames);
    ui->pluginList->setSortingEnabled(true);
    ui->pluginList->sortItems(Name, Qt::AscendingOrder);
    ui->pluginList->setRootIsDecorated(false);
    ui->pluginList->setItemsExpandable(false);

    initPluginList();
}

PluginSettingsDialog::~PluginSettingsDialog()
{
    delete ui;
}

void PluginSettingsDialog::initPluginList()
{
    ui->pluginList->clear();
    auto pluginLoader = PluginLoader::instance();
    auto pluginList = pluginLoader->getPluginList();
    for (auto it = pluginList.begin(); it != pluginList.end(); ++it) {
        QString &pluginId = (*it);
        auto info = pluginLoader->getPluginInfo(pluginId);
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->pluginList);
        item->setCheckState(Enabled,
                            m_device->isPluginEnabled(pluginId) ? Qt::Checked : Qt::Unchecked);
        item->setData(Enabled, Qt::UserRole, pluginId);
        item->setText(Name, info.name());
        item->setIcon(Name, QIcon::fromTheme(info.iconName()));
        item->setText(Description, info.description());

        ui->pluginList->addTopLevelItem(item);

        {
            QWidget *w = new QWidget(ui->pluginList);
            QHBoxLayout *layout = new QHBoxLayout(w);
            layout->setContentsMargins(0, 0, 0, 0);
            QPushButton *configBtn = new QPushButton(ui->pluginList);
            configBtn->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
            layout->addSpacerItem(
                new QSpacerItem(10, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
            layout->addWidget(configBtn);
            layout->addSpacerItem(
                new QSpacerItem(10, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

            ui->pluginList->setItemWidget(item, Configuration, w);
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

void PluginSettingsDialog::on_filterEdit_textChanged(const QString &text)
{
    m_delayFilterTimer->start();
}
