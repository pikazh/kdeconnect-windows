#include "batterypluginconfigdialog.h"
#include "core/plugins/pluginid.h"
#include "ui_batterypluginconfigdialog.h"

BatteryPluginConfigDialog::BatteryPluginConfigDialog(Device::Ptr dev, QWidget *parent)
    : QDialog(parent)
    , m_device(dev)
    , ui(new Ui::BatteryPluginConfigDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->thresholdEdit->setInputMask(QStringLiteral("99"));

    QObject::connect(ui->warningCB,
                     &QCheckBox::checkStateChanged,
                     ui->thresholdEdit,
                     [this](Qt::CheckState state) {
                         ui->thresholdEdit->setEnabled(state == Qt::Checked);
                     });

    QObject::connect(this,
                     &BatteryPluginConfigDialog::accepted,
                     this,
                     &BatteryPluginConfigDialog::saveConfig);

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

BatteryPluginConfigDialog::~BatteryPluginConfigDialog()
{
    delete ui;
}

void BatteryPluginConfigDialog::loadConfig()
{
    auto pluginConfigPtr = pluginConfig();
    bool showWarning = pluginConfigPtr->getBool(QStringLiteral("warning"), true);
    ui->warningCB->setChecked(showWarning);

    int threshold = pluginConfigPtr->getInt(QStringLiteral("threshold"), 15);
    ui->thresholdEdit->setText(QString::number(threshold));
    ui->thresholdEdit->setEnabled(showWarning);
}

KdeConnectPluginConfig::Ptr BatteryPluginConfigDialog::pluginConfig()
{
    QString pluginIdStr = pluginIdString(PluginId::BatteryMonitor);
    auto plugin = m_device->plugin(pluginIdStr);
    if (plugin != nullptr) {
        return plugin->config();
    } else {
        KdeConnectPluginConfig::Ptr config(new KdeConnectPluginConfig(m_device->id(), pluginIdStr));
        return config;
    }
}

void BatteryPluginConfigDialog::saveConfig()
{
    auto pluginConfigPtr = pluginConfig();
    bool showWarning = ui->warningCB->isChecked();
    pluginConfigPtr->set(QStringLiteral("warning"), showWarning);

    QString thresholdText = ui->thresholdEdit->text();
    bool ok = false;
    int threshold = thresholdText.toInt(&ok);
    if (ok)
        pluginConfigPtr->set(QStringLiteral("threshold"), threshold);

    pluginConfigPtr->notifyConfigChanged();
}
