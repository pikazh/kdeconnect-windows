#include "presenterpluginconfigdialog.h"
#include "core/plugins/pluginid.h"
#include "ui_presenterpluginconfigdialog.h"

PresenterPluginConfigDialog::PresenterPluginConfigDialog(Device::Ptr dev, QWidget *parent)
    : QDialog(parent)
    , m_device(dev)
    , ui(new Ui::PresenterPluginConfigDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    QObject::connect(this,
                     &PresenterPluginConfigDialog::accepted,
                     this,
                     &PresenterPluginConfigDialog::saveConfig);

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

PresenterPluginConfigDialog::~PresenterPluginConfigDialog()
{
    delete ui;
}

void PresenterPluginConfigDialog::loadConfig() {}

KdeConnectPluginConfig::Ptr PresenterPluginConfigDialog::pluginConfig()
{
    QString pluginIdStr = pluginIdString(PluginId::Presenter);
    auto plugin = m_device->plugin(pluginIdStr);
    if (plugin != nullptr) {
        return plugin->config();
    } else {
        KdeConnectPluginConfig::Ptr config(new KdeConnectPluginConfig(m_device->id(), pluginIdStr));
        return config;
    }
}

void PresenterPluginConfigDialog::saveConfig() {}
