/**
 * SPDX-FileCopyrightText: 2015 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef KDECONNECTCONFIG_H
#define KDECONNECTCONFIG_H

#include <QDir>
#include <QObject>
#include <QSslKey>

#include "deviceinfo.h"
#include "kdeconnectcore_export.h"

class QSslCertificate;

class KDECONNECTCORE_EXPORT KdeConnectConfig : public QObject
{
    Q_OBJECT
public:
    static KdeConnectConfig *instance();

    /*
     * App related info
     */

    QString style();
    void setStyle(const QString &style);
    QString language();
    void setLanguage(const QString &lan);

    /*
     * Our own info
     */

    QString deviceId();
    QString name();
    DeviceType deviceType();
    QSslKey privateKey();
    QSslCertificate certificate();
    DeviceInfo deviceInfo();
    QSsl::KeyAlgorithm privateKeyAlgorithm();
    QString privateKeyPath();
    QString certificatePath();

    void setName(const QString &name);

    /*
     * Trusted devices
     */

    QStringList trustedDevices(); // list of ids
    void removeTrustedDevice(const QString &id);
    void addTrustedDevice(const DeviceInfo &deviceInfo);
    void updateTrustedDeviceInfo(const DeviceInfo &deviceInfo);
    DeviceInfo getTrustedDevice(const QString &id);
    QSslCertificate getTrustedDeviceCertificate(const QString &id);
    int getTrustedDeviceProtocolVersion(const QString &id);

    void setDeviceProperty(const QString &deviceId, const QString &name, const QString &value);
    QString getDeviceProperty(const QString &deviceId, const QString &name, const QString &defaultValue = QString());

    // Custom devices
    void setCustomDevices(const QStringList &addresses);
    QStringList customDevices() const;

    /*
     * Paths for config files, there is no guarantee the directories already exist
     */
    QDir baseConfigDir();
    QDir deviceConfigDir(const QString &deviceId);
    QDir pluginConfigDir(const QString &deviceId, const QString &pluginName); // Used by KdeConnectPluginConfig

    QDir baseDataDir();
    QDir deviceDataDir(const QString &deviceId);

Q_SIGNALS:
    void deviceNameChanged(const QString &deviceName);

private:
    KdeConnectConfig();

    void loadOrGeneratePrivateKeyAndCertificate(const QString &keyPath, const QString &certPath);
    bool loadPrivateKey(const QString &path);
    bool loadCertificate(const QString &path);
    void generatePrivateKey(const QString &path);
    void generateCertificate(const QString &path);

    void removeAllTrustedDevices();

    struct KdeConnectConfigPrivate *d;
};

#endif
