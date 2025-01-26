#include "desktopdaemon.h"

#include <QCoreApplication>

DesktopDaemon::DesktopDaemon(QObject *parent)
: Daemon(parent)
{

}

void DesktopDaemon::askPairingConfirmation(Device *device)
{

}

void DesktopDaemon::reportError(const QString &title, const QString &description)
{

}

void DesktopDaemon::sendSimpleNotification(const QString &eventId, const QString &title, const QString &text, const QString &iconName)
{

}

void DesktopDaemon::quit()
{
    QCoreApplication::instance()->quit();
}
