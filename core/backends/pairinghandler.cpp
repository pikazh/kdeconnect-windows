/**
 * SPDX-FileCopyrightText: 2015 Albert Vaca Cintora <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "pairinghandler.h"
#include "core/core_debug.h"
#include "kdeconnectconfig.h"

#include <QSsl>

//#include <KLocalizedString>
const int ALLOWED_TIMESTAMP_TIME_DIFFERENCE_SECONDS = 1800; // 30 min

PairingHandler::PairingHandler(Device *parent, Device::PairState initialState)
    : QObject(parent)
    , m_device(parent)
    , m_pairState(initialState)
{
    m_pairingTimeout.setSingleShot(true);
    m_pairingTimeout.setInterval(pairingTimeoutMsec);
    connect(&m_pairingTimeout, &QTimer::timeout, this, &PairingHandler::pairingTimeout);
}

void PairingHandler::packetReceived(const NetworkPacket &np)
{
    m_pairingTimeout.stop();
    bool wantsPair = np.get<bool>(QStringLiteral("pair"));
    if (wantsPair) {
        switch (m_pairState) {
        case Device::PairState::Requested:
            pairingDone();
            break;
        case Device::PairState::RequestedByPeer:
            qCDebug(KDECONNECT_CORE) << "Ignoring second pairing request before the first one timed out";
            break;
        case Device::PairState::Paired:
        case Device::PairState::NotPaired:
            if (m_pairState == Device::PairState::Paired) {
                qWarning(KDECONNECT_CORE)
                    << "Received pairing request from a device we already trusted.";
                // It would be nice to auto-accept the pairing request here, but since the pairing accept and pairing request
                // messages are identical, this could create an infinite loop if both devices are "accepting" each other pairs.
                // Instead, unpair and handle as if "NotPaired".
                m_pairState = Device::PairState::NotPaired;
                Q_EMIT unpaired();
            }

            if (m_device->protocolVersion() >= 8) {
                m_pairingTimestamp = np.get<long>(QStringLiteral("timestamp"), -1L);
                if (m_pairingTimestamp == -1L) {
                    m_pairState = Device::PairState::NotPaired;
                    Q_EMIT pairingFailed("Invalid pairing request");
                    return;
                }
                long currentTimestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
                if (abs(m_pairingTimestamp - currentTimestamp)
                    > ALLOWED_TIMESTAMP_TIME_DIFFERENCE_SECONDS) {
                    m_pairState = Device::PairState::NotPaired;
                    Q_EMIT pairingFailed("Device clocks are out of sync");
                    return;
                }
            }

            m_pairState = Device::PairState::RequestedByPeer;
            m_pairingTimeout.start();
            Q_EMIT incomingPairRequest();
            break;
        }
    } else { // wantsPair == false
        qCDebug(KDECONNECT_CORE) << "Unpair request received";
        switch (m_pairState) {
        case Device::PairState::NotPaired:
            qCDebug(KDECONNECT_CORE) << "Ignoring unpair request for already unpaired device";
            break;
        case Device::PairState::Requested:       // We started pairing and got rejected
        case Device::PairState::RequestedByPeer: // They stared pairing, then cancelled
            m_pairState = Device::PairState::NotPaired;
            Q_EMIT pairingFailed("Canceled by other peer");
            break;
        case Device::PairState::Paired:
            m_pairState = Device::PairState::NotPaired;
            Q_EMIT unpaired();
            break;
        }
    }
}

bool PairingHandler::requestPairing()
{
    m_pairingTimeout.stop();

    if (m_pairState == Device::PairState::Paired) {
        qWarning(KDECONNECT_CORE) << m_device->name()
                                  << ": requestPairing was called on an already paired device.";
        Q_EMIT pairingFailed(QString("%1: Already paired").arg(m_device->name()));
        return false;
    }
    if (m_pairState == Device::PairState::RequestedByPeer) {
        qWarning(KDECONNECT_CORE)
            << m_device->name()
            << ": Pairing already started by the other end, accepting their request.";
        return acceptPairing();
    }

    if (!m_device->isReachable()) {
        Q_EMIT pairingFailed(QString("%1: Device not reachable").arg(m_device->name()));
        return false;
    }

    m_pairState = Device::PairState::Requested;

    m_pairingTimeout.start();

    m_pairingTimestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
    NetworkPacket np(PACKET_TYPE_PAIR,
                     {
                         {QStringLiteral("timestamp"), QVariant::fromValue(m_pairingTimestamp)},
                         {QStringLiteral("pair"), true},
                     });
    const bool success = m_device->sendPacket(np);
    if (!success) {
        m_pairingTimeout.stop();
        qWarning(KDECONNECT_CORE) << m_device->name() << ": Failed to send pair request packet.";
        m_pairState = Device::PairState::NotPaired;
        Q_EMIT pairingFailed(QString("%1: Device not reachable").arg(m_device->name()));
    }
    return success;
}

bool PairingHandler::acceptPairing()
{
    m_pairingTimeout.stop();
    NetworkPacket np(PACKET_TYPE_PAIR, {{QStringLiteral("pair"), true}});
    const bool success = m_device->sendPacket(np);
    if (success) {
        pairingDone();
    } else {
        qWarning(KDECONNECT_CORE) << "Failed to send packet accepting pairing";
        m_pairState = Device::PairState::NotPaired;
        Q_EMIT pairingFailed("Device not reachable");
    }
    return success;
}

void PairingHandler::cancelPairing()
{
    m_pairingTimeout.stop();
    m_pairState = Device::PairState::NotPaired;
    NetworkPacket np(PACKET_TYPE_PAIR, {{QStringLiteral("pair"), false}});
    m_device->sendPacket(np);
    Q_EMIT pairingFailed("Cancelled by user");
}

void PairingHandler::unpair()
{
    m_pairState = Device::PairState::NotPaired;
    NetworkPacket np(PACKET_TYPE_PAIR, {{QStringLiteral("pair"), false}});
    m_device->sendPacket(np);
    Q_EMIT unpaired();
}

void PairingHandler::pairingTimeout()
{
    NetworkPacket np(PACKET_TYPE_PAIR, {{QStringLiteral("pair"), false}});
    m_device->sendPacket(np);
    m_pairState = Device::PairState::NotPaired;
    Q_EMIT pairingFailed("Timed out");
}

void PairingHandler::pairingDone()
{
    qCDebug(KDECONNECT_CORE) << "Pairing done";
    m_pairState = Device::PairState::Paired;
    Q_EMIT pairingSuccessful();
}

QString PairingHandler::verificationKey() const
{
    auto a = KdeConnectConfig::instance().certificate().publicKey().toDer();
    auto b = m_device->certificate().publicKey().toDer();
    if (a < b) {
        std::swap(a, b);
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(a);
    hash.addData(b);

    if (m_device->protocolVersion() >= 8) {
        QByteArray timestamp;
        timestamp.setNum(m_pairingTimestamp);
        hash.addData(timestamp);
    }

    return QString::fromLatin1(hash.result().toHex().left(8).toUpper());
}
