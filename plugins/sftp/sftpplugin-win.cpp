/**
 * SPDX-FileCopyrightText: 2019 Piyush Aggarwal <piyushaggarwal002@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "sftpplugin-win.h"
#include "core/plugins/pluginfactory.h"
#include "plugin_sftp_debug.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>

K_PLUGIN_CLASS_WITH_JSON(SftpPlugin, "kdeconnect_sftp.json")

SftpPlugin::SftpPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
}

bool SftpPlugin::startBrowsing()
{
    NetworkPacket np(PACKET_TYPE_SFTP_REQUEST, {{QStringLiteral("startBrowsing"), true}});
    sendPacket(np);
    return false;
}

void SftpPlugin::receivePacket(const NetworkPacket &np)
{
    QStringList receivedFieldsList = np.body().keys();
    QSet<QString> receivedFields(receivedFieldsList.begin(), receivedFieldsList.end());
    if (!(expectedFields - receivedFields).isEmpty()) {
        qCWarning(KDECONNECT_PLUGIN_SFTP) << "Invalid packet received.";
        for (QString missingField : (expectedFields - receivedFields)) {
            qCWarning(KDECONNECT_PLUGIN_SFTP) << "Field" << missingField << "missing from packet.";
        }
        return;
    }
    if (np.has(QStringLiteral("errorMessage"))) {
        qCWarning(KDECONNECT_PLUGIN_SFTP) << np.get<QString>(QStringLiteral("errorMessage"));
        return;
    }

    QString path;
    if (np.has(QStringLiteral("multiPaths"))) {
        QStringList paths = np.get<QStringList>(QStringLiteral("multiPaths"));
        if (paths.size() == 1) {
            path = paths[0];
        } else {
            path = QStringLiteral("/");
        }
    } else {
        path = np.get<QString>(QStringLiteral("path"));
    }
    if (!path.endsWith(QChar::fromLatin1('/'))) {
        path += QChar::fromLatin1('/');
    }

    QString url_string = QStringLiteral("sftp://%1:%2@%3:%4%5")
                             .arg(np.get<QString>(QStringLiteral("user")),
                                  np.get<QString>(QStringLiteral("password")),
                                  np.get<QString>(QStringLiteral("ip")),
                                  np.get<QString>(QStringLiteral("port")),
                                  path);

    if (findScpExePath()) {
        QProcess::startDetached(m_scpExePath, {url_string});
    }
}

bool SftpPlugin::findScpExePath()
{
    if (m_scpExePath.isEmpty()) {
        QDirIterator winScpFileIt(QCoreApplication::applicationDirPath(),
                                  {"WinSCP.exe"},
                                  QDir::Files,
                                  QDirIterator::Subdirectories);
        while (winScpFileIt.hasNext()) {
            winScpFileIt.next();
            QString filePath = winScpFileIt.filePath();
            QFileInfo fileInfo(filePath);
            if (fileInfo.isExecutable()) {
                m_scpExePath = filePath;
                return true;
            }
        }

        return false;
    } else {
        return true;
    }
}

#include "sftpplugin-win.moc"
