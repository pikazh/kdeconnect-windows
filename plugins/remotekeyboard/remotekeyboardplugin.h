/**
 * SPDX-FileCopyrightText: 2017 Holger Kaelberer <holger.k@elberer.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "core/plugins/kdeconnectplugin.h"

#include <QVariantMap>

#define PACKET_TYPE_MOUSEPAD_REQUEST QLatin1String("kdeconnect.mousepad.request")
#define PACKET_TYPE_MOUSEPAD_ECHO QLatin1String("kdeconnect.mousepad.echo")
#define PACKET_TYPE_MOUSEPAD_KEYBOARDSTATE QLatin1String("kdeconnect.mousepad.keyboardstate")

class RemoteKeyboardPlugin : public KdeConnectPlugin
{
    Q_OBJECT

    Q_PROPERTY(bool remoteState READ remoteState NOTIFY remoteStateChanged)

public:
    explicit RemoteKeyboardPlugin(QObject *parent, const QVariantList &args);
    virtual ~RemoteKeyboardPlugin() override = default;

public Q_SLOTS:
    void sendKeyPress(const QString &key,
                      int specialKey = 0,
                      bool shift = false,
                      bool ctrl = false,
                      bool alt = false,
                      bool sendAck = false);
    void sendQKeyEvent(int key,
                       Qt::KeyboardModifiers modifiers,
                       const QString &text,
                       bool sendAck = false);
    int translateQtKey(int qtKey);

    bool remoteState() const { return m_remoteState; }

Q_SIGNALS:
    void keyPressEchoReceived(const QString &key,
                              int specialKey = 0,
                              bool shift = false,
                              bool ctrl = false,
                              bool alt = false);
    void remoteStateChanged(bool state);

protected:
    virtual void receivePacket(const NetworkPacket &np) override;

private:
    bool m_remoteState;
};
