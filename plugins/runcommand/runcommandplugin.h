/**
 * SPDX-FileCopyrightText: 2015 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "core/plugins/kdeconnectplugin.h"

class RunCommandPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    explicit RunCommandPlugin(QObject *parent, const QVariantList &args);

protected:
    virtual void receivePacket(const NetworkPacket &np) override;

protected Q_SLOTS:
    void sendCommandList();

Q_SIGNALS:
    void setup();
};
