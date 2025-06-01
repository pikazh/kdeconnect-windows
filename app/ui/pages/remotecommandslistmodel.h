#pragma once

#include "core/device.h"

#include "plugin/remotecommandspluginwrapper.h"

#include <QAbstractListModel>
#include <QList>

class RemoteCommandsListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    struct RemoteCommand
    {
        QString key;
        QString name;
        QString command;
    };

    explicit RemoteCommandsListModel(RemoteCommandsPluginWrapper *pluginWrapper,
                                     QObject *parent = nullptr);
    virtual ~RemoteCommandsListModel() = default;

    //virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    RemoteCommand at(size_t row);

protected Q_SLOTS:
    void pluginCommandsUpdated(const QByteArray &data);

private:
    QList<RemoteCommand> m_commandList;

    RemoteCommandsPluginWrapper *m_remoteCommandsPluginWrapper = nullptr;
};
