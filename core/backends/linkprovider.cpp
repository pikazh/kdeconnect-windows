/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "linkprovider.h"
#include "core/core_debug.h"

LinkProvider::LinkProvider()
{
}

void LinkProvider::suspend(bool suspend)
{
    if (suspend) {
        qCDebug(KDECONNECT_CORE) << "Stopping connection for suspension";
        onStop();
    } else {
        qCDebug(KDECONNECT_CORE) << "Restarting connection after suspension";
        onStart();
    }
}

