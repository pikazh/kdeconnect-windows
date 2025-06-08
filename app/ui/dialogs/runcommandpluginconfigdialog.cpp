#include "runcommandpluginconfigdialog.h"
#include "ui_runcommandpluginconfigdialog.h"

#include "app_debug.h"

#include "core/plugins/pluginid.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QTableWidgetItem>
#include <QUuid>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

RunCommandPluginConfigDialog::RunCommandPluginConfigDialog(Device::Ptr dev, QWidget *parent)
    : QDialog(parent)
    , m_device(dev)
    , ui(new Ui::RunCommandPluginConfigDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    QList<QString> commandTableHeaderText = {tr("Name"), tr("Command"), tr("")};
    ui->commandTable->setColumnCount(commandTableHeaderText.size());
    ui->commandTable->setColumnWidth(Columns::Command, 400);
    for (int i = 0; i < ui->commandTable->columnCount(); ++i) {
        ui->commandTable->setHorizontalHeaderItem(i,
                                                  new QTableWidgetItem(commandTableHeaderText[i]));
    }

    QMenu *defaultMenu = new QMenu(this);
#ifdef Q_OS_WIN
    addSuggestedCommand(defaultMenu, tr("Schedule a shutdown"), QStringLiteral("shutdown /s /t 60"));
    addSuggestedCommand(defaultMenu, tr("Shutdown now"), QStringLiteral("shutdown /s /t 0"));
    addSuggestedCommand(defaultMenu, tr("Cancel last shutdown"), QStringLiteral("shutdown /a"));
    addSuggestedCommand(defaultMenu, tr("Schedule a reboot"), QStringLiteral("shutdown /r /t 60"));
    addSuggestedCommand(defaultMenu,
                        tr("Suspend"),
                        QStringLiteral("rundll32.exe powrprof.dll,SetSuspendState 0,1,0"));
    addSuggestedCommand(defaultMenu,
                        tr("Lock Screen"),
                        QStringLiteral("rundll32.exe user32.dll,LockWorkStation"));
    addSuggestedCommand(defaultMenu,
                        tr("Say Hello"),
                        QStringLiteral(
                            "PowerShell -Command Add-Type -AssemblyName System.Speech; (New-Object "
                            "System.Speech.Synthesis.SpeechSynthesizer).Speak('hello');"));
#endif

    ui->addCommandsButton->setIcon(RETRIEVE_THEME_ICON("list-add"));
    ui->addCommandsButton->setMenu(defaultMenu);

    QObject::connect(this,
                     &RunCommandPluginConfigDialog::accepted,
                     this,
                     &RunCommandPluginConfigDialog::saveConfig);

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

    QObject::connect(ui->buttonBox,
                     &QDialogButtonBox::clicked,
                     this,
                     [this](QAbstractButton *button) {
                         if (button
                             == reinterpret_cast<QAbstractButton *>(
                                 ui->buttonBox->button(QDialogButtonBox::Reset))) {
                             deletAllRows();
                         }
                     });

    loadConfig();
}

RunCommandPluginConfigDialog::~RunCommandPluginConfigDialog()
{
    delete ui;
}

void RunCommandPluginConfigDialog::loadConfig()
{
    auto pluginConfigPtr = pluginConfig();

    QJsonDocument jsonDocument = QJsonDocument::fromJson(
        pluginConfigPtr->getByteArray(QStringLiteral("commands"), "{}"));
    QJsonObject commandList = jsonDocument.object();
    const auto keys = commandList.keys();
    for (auto &key : keys) {
        QJsonObject entry = commandList[key].toObject();
        const QString name = entry[QStringLiteral("name")].toString();
        const QString command = entry[QStringLiteral("command")].toString();
        if (name.isEmpty() || command.isEmpty())
            continue;

        addCommandToTable(name, command, key);
    }
}

KdeConnectPluginConfig::Ptr RunCommandPluginConfigDialog::pluginConfig()
{
    QString pluginIdStr = pluginIdString(PluginId::RunCommand);
    auto plugin = m_device->plugin(pluginIdStr);
    if (plugin != nullptr) {
        return plugin->config();
    } else {
        KdeConnectPluginConfig::Ptr config(new KdeConnectPluginConfig(m_device->id(), pluginIdStr));
        return config;
    }
}

void RunCommandPluginConfigDialog::addSuggestedCommand(QMenu *menu,
                                                       const QString &name,
                                                       const QString &command)
{
    auto action = new QAction(name);
    QObject::connect(action, &QAction::triggered, this, [this, name, command]() {
        addCommandToTable(name, command, QUuid::createUuid().toString(QUuid::WithoutBraces));
    });

    menu->addAction(action);
}

void RunCommandPluginConfigDialog::addCommandToTable(const QString &name,
                                                     const QString &command,
                                                     const QString &key)
{
    auto rowCount = ui->commandTable->rowCount();
    ui->commandTable->insertRow(rowCount);

    auto nameItem = new QTableWidgetItem(name);
    nameItem->setData(Qt::UserRole + 1, key);
    auto commandItem = new QTableWidgetItem(command);
    ui->commandTable->setItem(rowCount, Columns::Name, nameItem);
    ui->commandTable->setItem(rowCount, Columns::Command, commandItem);

    QWidget *w = new QWidget(ui->commandTable);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    QPushButton *deleteBtn = new QPushButton(w);
    deleteBtn->setProperty("nameItem", QVariant::fromValue(nameItem));
    deleteBtn->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout->addWidget(deleteBtn);
    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

    QObject::connect(
        deleteBtn,
        &QPushButton::clicked,
        this,
        [this]() {
            auto nameItem = qvariant_cast<QTableWidgetItem *>(
                QObject::sender()->property("nameItem"));
            Q_ASSERT(nameItem != nullptr);
            int index = ui->commandTable->row(nameItem);
            ui->commandTable->removeRow(index);
        },
        Qt::QueuedConnection);

    ui->commandTable->setCellWidget(rowCount, Columns::DeleteButton, w);
}

void RunCommandPluginConfigDialog::deletAllRows()
{
    for (int i = ui->commandTable->rowCount() - 1; i >= 0; --i) {
        ui->commandTable->removeRow(i);
    }
}

void RunCommandPluginConfigDialog::saveConfig()
{
    QJsonObject commands;
    for (int i = 0; i < ui->commandTable->rowCount(); i++) {
        const QString name = ui->commandTable->item(i, Columns::Name)->text();
        const QString key
            = ui->commandTable->item(i, Columns::Name)->data(Qt::UserRole + 1).toString();
        const QString command = ui->commandTable->item(i, Columns::Command)->text();

        if (name.isEmpty() || command.isEmpty() || key.isEmpty()) {
            continue;
        }

        QJsonObject entry;
        entry[QStringLiteral("name")] = name;
        entry[QStringLiteral("command")] = command;
        commands[key] = entry;
    }
    QJsonDocument document;
    document.setObject(commands);

    auto pluginConfigPtr = pluginConfig();
    pluginConfigPtr->set(QStringLiteral("commands"), document.toJson(QJsonDocument::Compact));

    pluginConfigPtr->notifyConfigChanged();
}

void RunCommandPluginConfigDialog::on_importButton_clicked()
{
    auto fileSelectDialog = new QFileDialog(this, tr("Import Commands"), QDir::homePath());
    fileSelectDialog->setNameFilters({"JSON (*.json)", "Any files (*)"});
    fileSelectDialog->setFileMode(QFileDialog::ExistingFile);
    if (fileSelectDialog->exec() != QDialog::Accepted) {
        return;
    }
    QStringList selected = fileSelectDialog->selectedFiles();
    if (selected.isEmpty())
        return;

    QFile file(selected[0]);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning(KDECONNECT_APP) << "Could not read file:" << selected[0];
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull() || !jsonDoc.isArray()) {
        qWarning(KDECONNECT_APP) << "Invalid JSON format.";
        return;
    }

    // Clear the current command list
    this->deletAllRows();

    // Populate the model with the imported commands
    QJsonArray jsonArray = jsonDoc.array();
    for (const QJsonValue &jsonValue : jsonArray) {
        QJsonObject jsonObj = jsonValue.toObject();
        QString name = jsonObj.value(QStringLiteral("name")).toString().trimmed();
        QString command = jsonObj.value(QStringLiteral("command")).toString().trimmed();
        if (name.isEmpty() || command.isEmpty())
            continue;

        addCommandToTable(name, command, QUuid::createUuid().toString(QUuid::WithoutBraces));
    }
}

void RunCommandPluginConfigDialog::on_exportButton_clicked()
{
    auto fileSelectDialog = new QFileDialog(this, tr("Export Commands"), QDir::homePath());
    fileSelectDialog->setNameFilters({"JSON (*.json)", "Any files (*)"});
    fileSelectDialog->setDefaultSuffix(QStringLiteral("json"));
    if (fileSelectDialog->exec() != QDialog::Accepted) {
        return;
    }

    QStringList selected = fileSelectDialog->selectedFiles();
    if (selected.isEmpty())
        return;

    QFile file(selected[0]);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qWarning(KDECONNECT_APP) << "Could not write to file:" << selected[0];
        return;
    }

    QJsonArray jsonArray;
    for (int i = 0; i < ui->commandTable->rowCount(); i++) {
        const QString name = ui->commandTable->item(i, Columns::Name)->text();
        const QString command = ui->commandTable->item(i, Columns::Command)->text();

        if (name.isEmpty() || command.isEmpty()) {
            continue;
        }

        QJsonObject entry;
        entry[QStringLiteral("name")] = name;
        entry[QStringLiteral("command")] = command;
        jsonArray.append(entry);
    }

    QJsonDocument jsonDocument(jsonArray);
    file.write(jsonDocument.toJson());
    file.close();
}
