#include "clipboardpluginconfigdialog.h"
#include "core/plugins/pluginid.h"
#include "ui_clipboardpluginconfigdialog.h"

ClipboardPluginConfigDialog::ClipboardPluginConfigDialog(Device::Ptr dev, QWidget *parent)
    : QDialog(parent)
    , m_device(dev)
    , ui(new Ui::ClipboardPluginConfigDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    QObject::connect(this,
                     &ClipboardPluginConfigDialog::accepted,
                     this,
                     &ClipboardPluginConfigDialog::saveConfig);

    QObject::connect(ui->buttonBox,
                     &QDialogButtonBox::clicked,
                     this,
                     [this](QAbstractButton *button) {
                         if (button
                             == reinterpret_cast<QAbstractButton *>(
                                 ui->buttonBox->button(QDialogButtonBox::Apply))) {
                             saveConfig();
                         }
                     });

    loadConfig();
}

ClipboardPluginConfigDialog::~ClipboardPluginConfigDialog()
{
    delete ui;
}

void ClipboardPluginConfigDialog::loadConfig()
{
    auto pluginConfigPtr = pluginConfig();

    bool autoSync = pluginConfigPtr->getBool(QStringLiteral("autoSync"), false);
    ui->autoSyncCB->setChecked(autoSync);

    bool sharePasswd = pluginConfigPtr->getBool(QStringLiteral("sendPassword"), false);
    ui->sharePasswordCB->setChecked(sharePasswd);
}

KdeConnectPluginConfig::Ptr ClipboardPluginConfigDialog::pluginConfig()
{
    QString pluginIdStr = pluginIdString(PluginId::ClipBoard);
    auto plugin = m_device->plugin(pluginIdStr);
    if (plugin != nullptr) {
        return plugin->config();
    } else {
        KdeConnectPluginConfig::Ptr config(new KdeConnectPluginConfig(m_device->id(), pluginIdStr));
        return config;
    }
}

void ClipboardPluginConfigDialog::saveConfig()
{
    auto pluginConfigPtr = pluginConfig();

    pluginConfigPtr->set(QStringLiteral("autoSync"), ui->autoSyncCB->isChecked());
    pluginConfigPtr->set(QStringLiteral("sendPassword"), ui->sharePasswordCB->isChecked());

    pluginConfigPtr->notifyConfigChanged();
}
