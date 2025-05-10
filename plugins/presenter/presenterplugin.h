/**
 * SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QTimer>

#include "core/plugins/kdeconnectplugin.h"

#define PACKET_TYPE_PRESENTER QStringLiteral("kdeconnect.presenter")

class PresenterView;

class PresenterPlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    explicit PresenterPlugin(QObject *parent, const QVariantList &args);

protected:
    virtual void receivePacket(const NetworkPacket &np) override;

private:
    QScopedPointer<PresenterView> m_view;
    QTimer *m_timer;
    qreal m_xPos, m_yPos;
};
