#include "dbushelper.h"

#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>

#include "core_debug.h"

const QString DBusHelper::dbus_daemon = QStringLiteral("dbus-daemon.exe");
const QString DBusHelper::kdeconnect_daemon = QStringLiteral("kdeconnectd.exe");
const QString DBusHelper::kdeconnect_app = QStringLiteral("kdeconnect-app.exe");

void DBusHelper::startDBusDaemon()
{
    const QString dbusPath =
        QCoreApplication::instance()->applicationDirPath() + QDir::separator() + dbus_daemon;

    if(!QProcess::startDetached(dbusPath, {QStringLiteral("--config-file=session.conf")}, QCoreApplication::instance()->applicationDirPath()))
    {
        qWarning(KDECONNECT_CORE) << "create process dbus-daemon failed.";
    }
}

void DBusHelper::terminalProcesses()
{

}

void DBusHelper::filterNonExportableCharacters(QString &src)
{
    QRegularExpression regexp(QStringLiteral("[^A-Za-z0-9_]"), QRegularExpression::CaseInsensitiveOption);
    src.replace(regexp, QLatin1String("_"));
}
