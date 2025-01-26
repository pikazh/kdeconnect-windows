#include "core/daemon.h"

#include <QObject>
#include <QString>

class DesktopDaemon : public Daemon
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.daemon")
public:
    explicit DesktopDaemon(QObject *parent = nullptr);

    virtual void askPairingConfirmation(Device *device) override;
    virtual void reportError(const QString &title, const QString &description) override;

    Q_SCRIPTABLE virtual void sendSimpleNotification(const QString &eventId, const QString &title, const QString &text, const QString &iconName) override;

public Q_SLOTS:
    virtual void quit() override;

};
