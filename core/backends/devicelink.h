/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DEVICELINK_H
#define DEVICELINK_H

#include <QObject>

#include "core/deviceinfo.h"
#include "core/networkpacket.h"

class LinkProvider;

class KDECONNECTCORE_EXPORT DeviceLink : public QObject
{
    Q_OBJECT
public:
    DeviceLink(const QString &deviceId, LinkProvider *parent);

    QString deviceId() const
    {
        return deviceInfo().id;
    }

    int priority() const
    {
        return priorityFromProvider;
    }

    virtual bool sendPacket(NetworkPacket &np) = 0;

    virtual DeviceInfo deviceInfo() const = 0;

private:
    int priorityFromProvider;

Q_SIGNALS:
    void receivedPacket(const NetworkPacket &np);
};

#endif
