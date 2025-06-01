/**
 * SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "core/plugins/kdeconnectplugin.h"

#include <QByteArray>
#include <QString>

class RemoteCommandsPlugin : public KdeConnectPlugin
{
    Q_OBJECT

    Q_PROPERTY(QByteArray commands READ commands NOTIFY commandsChanged)
    Q_PROPERTY(bool canAddCommand READ canAddCommand CONSTANT)

public:
    explicit RemoteCommandsPlugin(QObject *parent, const QVariantList &args);

    QByteArray commands() const
    {
        return m_commands;
    }

    bool canAddCommand() const { return m_canAddCommand; }

public Q_SLOTS:
    Q_SCRIPTABLE void triggerCommand(const QString &key);
    Q_SCRIPTABLE void editCommands();

protected:
    virtual void receivePacket(const NetworkPacket &np) override;
    virtual void onPluginEnabled() override;

Q_SIGNALS:
    Q_SCRIPTABLE void commandsChanged(const QByteArray &commands);

private:
    void setCommands(const QByteArray &commands);

    QByteArray m_commands;
    bool m_canAddCommand;
};
