/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "landevicelink.h"
#include "core/backends/linkprovider.h"
#include "core/core_debug.h"
#include "lanlinkprovider.h"

#include <QTimer>

LanDeviceLink::LanDeviceLink(const DeviceInfo &deviceInfo, LanLinkProvider *parent, QSslSocket *socket)
    : DeviceLink(deviceInfo.id, parent)
    , m_socket(nullptr)
    , m_deviceInfo(deviceInfo)
{
    reset(socket);
}

void LanDeviceLink::reset(QSslSocket *socket)
{
    if (m_socket) {
        disconnect(m_socket, &QAbstractSocket::disconnected, this, &QObject::deleteLater);

        // give some time to m_socket to handle not-written-yet and received data
        QObject::connect(m_socket, &QSslSocket::disconnected, m_socket, &QSslSocket::deleteLater);
        QTimer *delayDeleteTimer = new QTimer(m_socket);
        QSslSocket *dalayDeleteSocket = m_socket;
        QObject::connect(delayDeleteTimer, &QTimer::timeout, this, [dalayDeleteSocket]() {
            dalayDeleteSocket->disconnectFromHost();
        });
        delayDeleteTimer->setSingleShot(true);
        delayDeleteTimer->setInterval(10 * 1000);
        delayDeleteTimer->start();
    }

    m_socket = socket;
    socket->setParent(this);

    connect(socket, &QAbstractSocket::disconnected, this, &QObject::deleteLater);
    connect(socket, &QAbstractSocket::readyRead, this, &LanDeviceLink::dataReceived);
}

QHostAddress LanDeviceLink::hostAddress() const
{
    if (!m_socket) {
        return QHostAddress::Null;
    }
    QHostAddress addr = m_socket->peerAddress();
    if (addr.protocol() == QAbstractSocket::IPv6Protocol) {
        bool success;
        QHostAddress convertedAddr = QHostAddress(addr.toIPv4Address(&success));
        if (success) {
            qCDebug(KDECONNECT_CORE) << "Converting IPv6" << addr << "to IPv4" << convertedAddr;
            addr = convertedAddr;
        }
    }
    return addr;
}

bool LanDeviceLink::sendPacket(NetworkPacket &np)
{
    int written = m_socket->write(np.serialize());

    // Actually we can't detect if a packet is received or not. We keep TCP
    //"ESTABLISHED" connections that look legit (return true when we use them),
    // but that are actually broken (until keepalive detects that they are down).
    return (written != -1);
    /*
    if (np.payload()) {
        // Device *device = Daemon::instance()->getDevice(deviceId());
        // if (device == nullptr) {
        //     qCWarning(KDECONNECT_CORE) << "Device disconnected" << deviceId();
        //     return false;
        // }
        // FIXME: Remove packet-type-specific logic from the link
        // if (np.type() == PACKET_TYPE_SHARE_REQUEST && np.payloadSize() >= 0) {
        //     if (!m_compositeUploadJob || !m_compositeUploadJob->isRunning()) {
        //         m_compositeUploadJob = new CompositeUploadJob(device, true);
        //     }

        //     m_compositeUploadJob->addSubjob(new UploadJob(np));

        //     if (!m_compositeUploadJob->isRunning()) {
        //         m_compositeUploadJob->start();
        //     }
        // } else { // Infinite stream
        //     CompositeUploadJob *fireAndForgetJob = new CompositeUploadJob(device, false);
        //     fireAndForgetJob->addSubjob(new UploadJob(np));
        //     fireAndForgetJob->start();
        // }

        return true;
    } else {
        
    }
*/
}

void LanDeviceLink::dataReceived()
{
    QSslSocket *socket = qobject_cast<QSslSocket *>(QObject::sender());
    if (socket == nullptr)
        return;

    while (socket->canReadLine()) {
        const QByteArray serializedPacket = socket->readLine();
        NetworkPacket packet;
        NetworkPacket::unserialize(serializedPacket, &packet);

        // qCDebug(KDECONNECT_CORE) << "LanDeviceLink dataReceived" << serializedPacket;

        if (packet.hasPayloadTransferInfo()) {
            QVariantMap transferInfo = packet.payloadTransferInfo();
            QString address = socket->peerAddress().toString();
            //const quint16 port = transferInfo[QStringLiteral("port")].toInt();
            transferInfo[QStringLiteral("host")] = address;
            packet.setPayloadTransferInfo(transferInfo);
        }

        Q_EMIT receivedPacket(packet);
    }
}
