#include "remotecommandspage.h"
#include "ui_remotecommandspage.h"

#include "application.h"
#include "remotecommandslistitemdelegate.h"
#include "remotecommandslistmodel.h"

#include <QIcon>

RemoteCommandsPage::RemoteCommandsPage(Device::Ptr device, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RemoteCommandsPage)
    , m_remoteCommandPluginWrapper(new RemoteCommandsPluginWrapper(device, this))
{
    ui->setupUi(this);
    ui->editCommandsButton->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    m_remoteCommandPluginWrapper->init();
    // do this only after pluginWrapper->init()
    m_remoteCommandsListModel = new RemoteCommandsListModel(m_remoteCommandPluginWrapper,
                                                            ui->commandsList);

    ui->commandsList->setModel(m_remoteCommandsListModel);
    ui->commandsList->setItemDelegate(new RemoteCommandsListItemDelegate(ui->commandsList));
}

RemoteCommandsPage::~RemoteCommandsPage()
{
    delete ui;
}

QIcon RemoteCommandsPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("system-run"));
}

bool RemoteCommandsPage::apply()
{
    return true;
}

bool RemoteCommandsPage::shouldDisplay() const
{
    return m_remoteCommandPluginWrapper->isPluginLoaded();
}

void RemoteCommandsPage::retranslate()
{
    ui->retranslateUi(this);
}

void RemoteCommandsPage::on_commandsList_activated(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    RemoteCommandsListModel::RemoteCommand remoteCommand = m_remoteCommandsListModel->at(
        index.row());
    m_remoteCommandPluginWrapper->triggerCommand(remoteCommand.key);
}

void RemoteCommandsPage::on_editCommandsButton_clicked()
{
    if (m_remoteCommandPluginWrapper->canAddCommand()) {
        m_remoteCommandPluginWrapper->editCommands();
        APPLICATION->showSystemTrayBalloonMessage(
            tr("You can edit commands on the connected device"));
    }
}
