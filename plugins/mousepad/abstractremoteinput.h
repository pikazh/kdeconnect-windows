/**
 * SPDX-FileCopyrightText: 2018 Albert Vaca Cintora <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

#include "core/networkpacket.h"
#include "plugin_mousepad_debug.h"

class AbstractRemoteInput : public QObject
{
    friend class MousepadPlugin;
    Q_OBJECT
public:
    explicit AbstractRemoteInput(QObject *parent = nullptr)
        : QObject(parent)
    {}

protected:
    virtual bool handlePacket(const NetworkPacket &np) = 0;
    virtual bool hasKeyboardSupport() { return false; }
};
