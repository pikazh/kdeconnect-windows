/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "shareplugin.h"
#include "plugin_share_debug.h"

#include "core/plugins/pluginfactory.h"
#include "notification.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QGuiApplication>
#include <QTemporaryFile>

K_PLUGIN_CLASS_WITH_JSON(SharePlugin, "kdeconnect_share.json")

SharePlugin::SharePlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
    Q_ASSERT(m_tempDir.isValid());
}

void SharePlugin::receivePacket(const NetworkPacket &np)
{
    if (np.has(QStringLiteral("filename"))) {
        handleSharedFile(np);
    } else if (np.has(QStringLiteral("text"))) {
        handleSharedText(np);
    } else if (np.has(QStringLiteral("url"))) {
        QUrl url = QUrl::fromEncoded(np.get<QByteArray>(QStringLiteral("url")));
        QDesktopServices::openUrl(url);
        //Q_EMIT shareReceived(url.toString());
    }
}

void SharePlugin::handleSharedText(const NetworkPacket &np)
{
    QString text = np.get<QString>(QStringLiteral("text"));

    QGuiApplication::clipboard()->setText(text);

    QString title = tr("Shared text from %1 copied to clipboard").arg(device()->name());
    auto notif = Notification::exec(QStringLiteral("KDE Connect"), title);
    auto *textEditorAction = notif->addAction(tr("Open in Text Editor"));
    auto openTextEditor = [this, text] {
        QTemporaryFile tmpFile;
        tmpFile.setFileTemplate(m_tempDir.path() + QDir::separator()
                                + QStringLiteral("kdeconnect-XXXXXX.txt"));
        tmpFile.setAutoRemove(false);
        tmpFile.open();
        tmpFile.write(text.toUtf8());
        tmpFile.close();

        const QString fileName = tmpFile.fileName();
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    };
    connect(textEditorAction, &NotificationAction::activated, this, openTextEditor);

    //Q_EMIT shareReceived(fileName);
}

void SharePlugin::handleSharedFile(const NetworkPacket &np) {}

void SharePlugin::shareText(const QString &text)
{
    NetworkPacket packet(PACKET_TYPE_SHARE_REQUEST);
    packet.set<QString>(QStringLiteral("text"), text);
    sendPacket(packet);
}

void SharePlugin::shareUrl(const QUrl &url)
{
    if (!url.isRelative() && !url.isLocalFile()) {
        NetworkPacket packet(PACKET_TYPE_SHARE_REQUEST);
        packet.set<QString>(QStringLiteral("url"), url.toString());
        sendPacket(packet);
    }
}

#include "shareplugin.moc"
