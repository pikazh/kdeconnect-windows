#include "remotecommandslistmodel.h"
#include "uicommon.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QSize>

const int g_itemHeight = 40;

RemoteCommandsListModel::RemoteCommandsListModel(RemoteCommandsPluginWrapper *pluginWrapper,
                                                 QObject *parent)
    : QAbstractListModel{parent}
    , m_remoteCommandsPluginWrapper(pluginWrapper)
{
    QObject::connect(m_remoteCommandsPluginWrapper,
                     &RemoteCommandsPluginWrapper::commandsChanged,
                     this,
                     &RemoteCommandsListModel::pluginCommandsUpdated);

    pluginCommandsUpdated(m_remoteCommandsPluginWrapper->commands());
}

int RemoteCommandsListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_commandList.size();
    }
}

QVariant RemoteCommandsListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    int column = index.column();
    if (role == Qt::ToolTipRole) {
        auto &cmd = m_commandList.at(index.row());
        return tr("Name: ") + cmd.name + QLatin1StringView("\n") + tr("Command: ") + cmd.command;
    } else if (role == Qt::SizeHintRole) {
        return QSize(0, g_itemHeight);
    } else if (role == RemoteCommandsListItemDataRoles::Name) {
        auto &cmd = m_commandList.at(index.row());
        return cmd.name;
    } else if (role == RemoteCommandsListItemDataRoles::Command) {
        auto &cmd = m_commandList.at(index.row());
        return cmd.command;
    } else if (role == RemoteCommandsListItemDataRoles::Key) {
        auto &cmd = m_commandList.at(index.row());
        return cmd.key;
    }

    return {};
}

void RemoteCommandsListModel::pluginCommandsUpdated(const QByteArray &data)
{
    QList<RemoteCommand> updatedCmds;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    QJsonObject commandList = jsonDocument.object();
    const auto keys = commandList.keys();
    for (auto &key : keys) {
        QJsonObject entry = commandList[key].toObject();

        RemoteCommand remoteCommand;
        remoteCommand.key = key;
        remoteCommand.name = entry[QStringLiteral("name")].toString();
        remoteCommand.command = entry[QStringLiteral("command")].toString();

        if (remoteCommand.name.isEmpty() || remoteCommand.command.isEmpty()
            || remoteCommand.key.isEmpty())
            continue;

        updatedCmds.push_back(remoteCommand);
    }

    beginResetModel();
    m_commandList = updatedCmds;
    endResetModel();
}

RemoteCommandsListModel::RemoteCommand RemoteCommandsListModel::at(size_t row)
{
    return m_commandList.at(row);
}
