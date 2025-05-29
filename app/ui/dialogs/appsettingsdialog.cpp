#include "appsettingsdialog.h"
#include "ui_appsettingsdialog.h"

#include "app_debug.h"

#include "core/kdeconnectconfig.h"

#include <QApplication>
#include <QList>
#include <QStringList>
#include <QStyle>
#include <QStyleFactory>

AppSettingsDialog::AppSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AppSettingsDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    QObject::connect(this, &AppSettingsDialog::accepted, this, &AppSettingsDialog::saveConfig);

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

    auto styleNames = QStyleFactory::keys();
    ui->styleComboBox->addItems(styleNames);

    auto currentStyleName = QApplication::style()->name();
    ui->styleComboBox->setCurrentText(currentStyleName);

    auto localDeviceName = KdeConnectConfig::instance()->name();
    ui->deviceNameLineEdit->setText(localDeviceName);
}

AppSettingsDialog::~AppSettingsDialog()
{
    delete ui;
}

void AppSettingsDialog::loadConfig()
{
    auto config = KdeConnectConfig::instance();
}

void AppSettingsDialog::saveConfig()
{
    auto config = KdeConnectConfig::instance();
    auto selectedStyleName = ui->styleComboBox->currentText();
    if (QApplication::style()->name().compare(selectedStyleName, Qt::CaseInsensitive) != 0) {
        auto newStyle = QStyleFactory::create(selectedStyleName);
        if (newStyle != nullptr) {
            QApplication::setStyle(newStyle);
            config->setStyle(selectedStyleName);
        } else {
            qWarning(KDECONNECT_APP)
                << "selected style:" << selectedStyleName << "is not valid style name";
        }
    }

    auto localDeviceName = KdeConnectConfig::instance()->name();
    QString editDeviceName = ui->deviceNameLineEdit->text().trimmed();
    if (!editDeviceName.isEmpty() && editDeviceName != localDeviceName) {
        QString filteredName = DeviceInfo::filterName(editDeviceName);
        if (!filteredName.isEmpty()) {
            ui->deviceNameLineEdit->setText(filteredName);
            KdeConnectConfig::instance()->setName(filteredName);
        }
    }
}
