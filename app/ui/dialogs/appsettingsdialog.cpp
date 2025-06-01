#include "appsettingsdialog.h"
#include "application.h"
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
    ui->languageChangedTipsLabel->hide();

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
    // on windows, QApplication::style()->name() returns the same name with that included in QStyleFactory::keys(),
    // but they don't match case
    for (auto &style : styleNames) {
        if (style.compare(currentStyleName, Qt::CaseInsensitive) == 0)
            ui->styleComboBox->setCurrentText(style);
    }

    auto localDeviceName = KdeConnectConfig::instance()->name();
    ui->deviceNameLineEdit->setText(localDeviceName);

    loadConfig();

    QObject::connect(ui->languageComboBox,
                     &QComboBox::currentIndexChanged,
                     ui->languageChangedTipsLabel,
                     &QLabel::show);
}

AppSettingsDialog::~AppSettingsDialog()
{
    delete ui;
}

void AppSettingsDialog::loadConfig()
{
    auto config = KdeConnectConfig::instance();
    auto language = APPLICATION->language();
    QStringList localeNames = language->localeNames();
    for (auto &localeName : localeNames) {
        ui->languageComboBox->addItem(QLocale(localeName).nativeLanguageName(), localeName);
    }

    ui->languageComboBox->setCurrentText(
        QLocale(language->currentLocaleName()).nativeLanguageName());
}

void AppSettingsDialog::changeEvent(QEvent *evt)
{
    if (evt->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    __super::changeEvent(evt);
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

    QString selectedLocaleName = ui->languageComboBox->currentData().toString();
    if (APPLICATION->language()->loadTranslation(selectedLocaleName)) {
        config->setLanguage(selectedLocaleName);
    }

    auto localDeviceName = KdeConnectConfig::instance()->name();
    QString editDeviceName = ui->deviceNameLineEdit->text().trimmed();
    if (!editDeviceName.isEmpty() && editDeviceName != localDeviceName) {
        QString filteredName = DeviceInfo::filterName(editDeviceName);
        if (!filteredName.isEmpty()) {
            ui->deviceNameLineEdit->setText(filteredName);
            config->setName(filteredName);
        }
    }
}
