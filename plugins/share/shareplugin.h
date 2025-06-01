/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "core/plugins/kdeconnectplugin.h"

#include <QTemporaryDir>
#include <QUrl>

#define PACKET_TYPE_SHARE_REQUEST QStringLiteral("kdeconnect.share.request")
#define PACKET_TYPE_SHARE_REQUEST_UPDATE QStringLiteral("kdeconnect.share.request.update")

class SharePlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    explicit SharePlugin(QObject *parent, const QVariantList &args);

public Q_SLOTS:
    void shareText(const QString &text);
    void shareUrl(const QUrl &url);

protected:
    virtual void receivePacket(const NetworkPacket &np) override;

    void handleSharedText(const NetworkPacket &np);
    void handleSharedFile(const NetworkPacket &np);

private:
    QTemporaryDir m_tempDir;
};
