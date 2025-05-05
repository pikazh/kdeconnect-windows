/**
 * SPDX-FileCopyrightText: 2019 Piyush Aggarwal <piyushaggarwal002@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "core/device.h"
#include "core/plugins/kdeconnectplugin.h"

#define PACKET_TYPE_SFTP_REQUEST QStringLiteral("kdeconnect.sftp.request")

static const QSet<QString> expectedFields = QSet<QString>()
    << QStringLiteral("ip") << QStringLiteral("port") << QStringLiteral("user") << QStringLiteral("password") << QStringLiteral("path");
;
class SftpPlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    explicit SftpPlugin(QObject *parent, const QVariantList &args);

protected:
    virtual void receivePacket(const NetworkPacket &np) override;

    bool findScpExePath();

public Q_SLOTS:
    bool startBrowsing();

private:
    QString m_scpExePath;
};
