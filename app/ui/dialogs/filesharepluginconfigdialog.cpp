#include "filesharepluginconfigdialog.h"
#include "ui_filesharepluginconfigdialog.h"

#include "core/plugins/pluginid.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

FileSharePluginConfigDialog::FileSharePluginConfigDialog(Device::Ptr dev, QWidget *parent)
    : QDialog(parent)
    , m_device(dev)
    , ui(new Ui::FileSharePluginConfigDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    QObject::connect(ui->selectDirButton, &QPushButton::clicked, this, [this]() {
        auto dirSelectDialog = new QFileDialog(this);
        dirSelectDialog->setFileMode(QFileDialog::Directory);
        dirSelectDialog->setOption(QFileDialog::ShowDirsOnly);
        if (dirSelectDialog->exec() == QDialog::Accepted) {
            auto selectedFiles = dirSelectDialog->selectedFiles();
            if (selectedFiles.size() > 0) {
                ui->filesavePathEdit->setText(selectedFiles[0]);
            }
        }
    });

    QObject::connect(this,
                     &FileSharePluginConfigDialog::accepted,
                     this,
                     &FileSharePluginConfigDialog::saveConfig);

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

FileSharePluginConfigDialog::~FileSharePluginConfigDialog()
{
    delete ui;
}

void FileSharePluginConfigDialog::loadConfig()
{
    auto pluginConfigPtr = pluginConfig();

    const QString defaultDownloadPath = QStandardPaths::writableLocation(
        QStandardPaths::DownloadLocation);
    QString savePath = pluginConfigPtr->getString(QStringLiteral("incoming_path"), "");
    QFileInfo fileInfo(savePath);
    if (fileInfo.isAbsolute() && QDir().mkpath(savePath)) {
        ui->filesavePathEdit->setText(savePath);
    } else {
        ui->filesavePathEdit->setText(defaultDownloadPath);
    }
}

KdeConnectPluginConfig::Ptr FileSharePluginConfigDialog::pluginConfig()
{
    QString pluginIdStr = pluginIdString(PluginId::Share);
    auto plugin = m_device->plugin(pluginIdStr);
    if (plugin != nullptr) {
        return plugin->config();
    } else {
        KdeConnectPluginConfig::Ptr config(new KdeConnectPluginConfig(m_device->id(), pluginIdStr));
        return config;
    }
}

void FileSharePluginConfigDialog::saveConfig()
{
    auto pluginConfigPtr = pluginConfig();

    const QString defaultDownloadPath = QStandardPaths::writableLocation(
        QStandardPaths::DownloadLocation);
    QString savePath = ui->filesavePathEdit->text().trimmed();
    QFileInfo fileInfo(savePath);
    if (fileInfo.isAbsolute() && QDir().mkpath(savePath)) {
        pluginConfigPtr->set(QStringLiteral("incoming_path"), savePath);
    } else {
        pluginConfigPtr->set(QStringLiteral("incoming_path"), defaultDownloadPath);
    }

    pluginConfigPtr->notifyConfigChanged();
}
