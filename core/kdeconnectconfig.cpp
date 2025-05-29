/**
 * SPDX-FileCopyrightText: 2015 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "kdeconnectconfig.h"
#include "core_debug.h"
#include "deviceinfo.h"
#include "plugins/pluginloader.h"
#include "sslhelper.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHostInfo>
#include <QLatin1String>
#include <QSettings>
#include <QSslCertificate>
#include <QStandardPaths>
#include <QThread>
#include <QUuid>

const QFile::Permissions strictPermissions = QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser;

const QString appDirName = QLatin1String("kdeconnectx");

struct KdeConnectConfigPrivate {
    QSslKey m_privateKey;
    QSslCertificate m_certificate;

    QSettings *m_config;
    QSettings *m_trustedDevices;
};

static QString getDefaultDeviceName()
{
    return QHostInfo::localHostName();
}

KdeConnectConfig *KdeConnectConfig::instance()
{
    static KdeConnectConfig kcc;
    return &kcc;
}

QString KdeConnectConfig::style()
{
    return d->m_config->value(QStringLiteral("style")).toString();
}

void KdeConnectConfig::setStyle(const QString &style)
{
    d->m_config->setValue(QStringLiteral("style"), style);
}

QString KdeConnectConfig::language()
{
    return d->m_config->value(QStringLiteral("language")).toString();
}

void KdeConnectConfig::setLanguage(const QString &lan)
{
    d->m_config->setValue(QStringLiteral("language"), lan);
}

KdeConnectConfig::KdeConnectConfig()
    : d(new KdeConnectConfigPrivate)
{
    // Make sure base directory exists
    QDir().mkpath(baseConfigDir().path());

    //.config/kdeconnect/config
    d->m_config = new QSettings(baseConfigDir().absoluteFilePath(QStringLiteral("config")), QSettings::IniFormat);
    d->m_trustedDevices = new QSettings(baseConfigDir().absoluteFilePath(QStringLiteral("trusted_devices")), QSettings::IniFormat);

    loadOrGeneratePrivateKeyAndCertificate(privateKeyPath(), certificatePath());

    if (name().isEmpty()) {
        setName(getDefaultDeviceName());
    }
}

QString KdeConnectConfig::name()
{
    return d->m_config->value(QStringLiteral("name")).toString();
}

void KdeConnectConfig::setName(const QString &name)
{
    d->m_config->setValue(QStringLiteral("name"), name);

    Q_EMIT deviceNameChanged(name);
}

DeviceType KdeConnectConfig::deviceType()
{
    const QByteArrayList platforms = qgetenv("PLASMA_PLATFORM").split(':');

    if (platforms.contains("phone"))
    {
        return DeviceType::Phone;
    }
    else if (platforms.contains("tablet"))
    {
        return DeviceType::Tablet;
    }
    else if (platforms.contains("mediacenter"))
    {
        return DeviceType::Tv;
    }

    // TODO non-Plasma mobile platforms

    return DeviceType::Desktop;
}

QString KdeConnectConfig::deviceId()
{
    return d->m_certificate.subjectInfo(QSslCertificate::CommonName).constFirst();
}

QString KdeConnectConfig::privateKeyPath()
{
    return baseConfigDir().absoluteFilePath(QStringLiteral("privateKey.pem"));
}

QString KdeConnectConfig::certificatePath()
{
    return baseConfigDir().absoluteFilePath(QStringLiteral("certificate.pem"));
}

QSslCertificate KdeConnectConfig::certificate()
{
    return d->m_certificate;
}

QSslKey KdeConnectConfig::privateKey()
{
    return d->m_privateKey;
}

DeviceInfo KdeConnectConfig::deviceInfo()
{
    const auto incoming = PluginLoader::instance()->incomingCapabilities();
    const auto outgoing = PluginLoader::instance()->outgoingCapabilities();
    return DeviceInfo(deviceId(),
                      certificate(),
                      name(),
                      deviceType(),
                      NetworkPacket::s_protocolVersion,
                      QSet(incoming.begin(), incoming.end()),
                      QSet(outgoing.begin(), outgoing.end()));
}

QSsl::KeyAlgorithm KdeConnectConfig::privateKeyAlgorithm()
{
    QString value = d->m_config->value(QStringLiteral("keyAlgorithm"), QStringLiteral("RSA")).toString();
    if (value == QLatin1String("EC")) {
        return QSsl::KeyAlgorithm::Ec;
    } else {
        return QSsl::KeyAlgorithm::Rsa;
    }
}

QDir KdeConnectConfig::baseConfigDir()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QString kdeconnectConfigPath = QDir(configPath).absoluteFilePath(appDirName);
    return QDir(kdeconnectConfigPath);
}

QStringList KdeConnectConfig::trustedDevices()
{
    const QStringList &list = d->m_trustedDevices->childGroups();
    return list;
}

void KdeConnectConfig::addTrustedDevice(const DeviceInfo &deviceInfo)
{
    d->m_trustedDevices->beginGroup(deviceInfo.id);
    d->m_trustedDevices->setValue(QStringLiteral("name"), deviceInfo.name);
    d->m_trustedDevices->setValue(QStringLiteral("type"), deviceInfo.type.toString());
    d->m_trustedDevices->setValue(QStringLiteral("protocolVersion"), deviceInfo.protocolVersion);
    QString certString = QString::fromLatin1(deviceInfo.certificate.toPem());
    d->m_trustedDevices->setValue(QStringLiteral("certificate"), certString);
    d->m_trustedDevices->endGroup();
    d->m_trustedDevices->sync();

    QDir().mkpath(deviceConfigDir(deviceInfo.id).path());
}

void KdeConnectConfig::updateTrustedDeviceInfo(const DeviceInfo &deviceInfo)
{
    if (!trustedDevices().contains(deviceInfo.id)) {
        qCWarning(KDECONNECT_CORE)
            << "trying to update untrusted device info. device name:" << deviceInfo.name;
        // do not store values for untrusted devices (it would make them trusted)
        return;
    }

    d->m_trustedDevices->beginGroup(deviceInfo.id);
    d->m_trustedDevices->setValue(QStringLiteral("name"), deviceInfo.name);
    d->m_trustedDevices->setValue(QStringLiteral("type"), deviceInfo.type.toString());
    d->m_trustedDevices->setValue(QStringLiteral("protocolVersion"), deviceInfo.protocolVersion);
    d->m_trustedDevices->endGroup();
    d->m_trustedDevices->sync();
}

QSslCertificate KdeConnectConfig::getTrustedDeviceCertificate(const QString &id)
{
    d->m_trustedDevices->beginGroup(id);
    QString certString = d->m_trustedDevices->value(QStringLiteral("certificate"), QString()).toString();
    d->m_trustedDevices->endGroup();
    return QSslCertificate(certString.toLatin1());
}

int KdeConnectConfig::getTrustedDeviceProtocolVersion(const QString &id)
{
    d->m_trustedDevices->beginGroup(id);
    int protocolVersion = d->m_trustedDevices->value(QStringLiteral("protocolVersion"), 0).toInt();
    d->m_trustedDevices->endGroup();
    return protocolVersion;
}

DeviceInfo KdeConnectConfig::getTrustedDevice(const QString &id)
{
    d->m_trustedDevices->beginGroup(id);

    QString certString = d->m_trustedDevices->value(QStringLiteral("certificate"), QString()).toString();
    QSslCertificate certificate(certString.toLatin1());
    QString name = d->m_trustedDevices->value(QStringLiteral("name"), QLatin1String("unnamed")).toString();
    QString typeString = d->m_trustedDevices->value(QStringLiteral("type"), QLatin1String("unknown")).toString();
    DeviceType type = DeviceType::FromString(typeString);

    d->m_trustedDevices->endGroup();

    return DeviceInfo(id, certificate, name, type);
}

void KdeConnectConfig::removeTrustedDevice(const QString &deviceId)
{
    d->m_trustedDevices->remove(deviceId);
    d->m_trustedDevices->sync();
    // We do not remove the config files.
}

void KdeConnectConfig::removeAllTrustedDevices()
{
    d->m_trustedDevices->clear();
    d->m_trustedDevices->sync();
}

// Utility functions to set and get a value
void KdeConnectConfig::setDeviceProperty(const QString &deviceId, const QString &key, const QString &value)
{
    // do not store values for untrusted devices (it would make them trusted)
    if (!trustedDevices().contains(deviceId)) {
        qCWarning(KDECONNECT_CORE)
            << "trying to set device property for untrusted device. device id:" << deviceId;
        return;
    }

    d->m_trustedDevices->beginGroup(deviceId);
    d->m_trustedDevices->setValue(key, value);
    d->m_trustedDevices->endGroup();
    d->m_trustedDevices->sync();
}

QString KdeConnectConfig::getDeviceProperty(const QString &deviceId, const QString &key, const QString &defaultValue)
{
    QString value;
    d->m_trustedDevices->beginGroup(deviceId);
    value = d->m_trustedDevices->value(key, defaultValue).toString();
    d->m_trustedDevices->endGroup();
    return value;
}

void KdeConnectConfig::setCustomDevices(const QStringList &addresses)
{
    d->m_config->setValue(QStringLiteral("customDevices"), addresses);
    d->m_config->sync();
}

QStringList KdeConnectConfig::customDevices() const
{
    return d->m_config->value(QStringLiteral("customDevices")).toStringList();
}

QDir KdeConnectConfig::deviceConfigDir(const QString &deviceId)
{
    QString deviceConfigPath = baseConfigDir().absoluteFilePath(deviceId);
    return QDir(deviceConfigPath);
}

QDir KdeConnectConfig::pluginConfigDir(const QString &deviceId, const QString &pluginId)
{
    QString deviceConfigPath = baseConfigDir().absoluteFilePath(deviceId);
    QString pluginConfigDir = QDir(deviceConfigPath).absoluteFilePath(pluginId);
    return QDir(pluginConfigDir);
}

QDir KdeConnectConfig::baseDataDir()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QString kdeconnectConfigPath = QDir(configPath).absoluteFilePath(appDirName);
    return QDir(kdeconnectConfigPath);
}

QDir KdeConnectConfig::deviceDataDir(const QString &deviceId)
{
    QString deviceDataPath = baseDataDir().absoluteFilePath(deviceId);
    return QDir(deviceDataPath);
}

bool KdeConnectConfig::loadPrivateKey(const QString &keyPath)
{
    QFile privKey(keyPath);
    if (privKey.exists() && privKey.open(QIODevice::ReadOnly)) {
        d->m_privateKey = QSslKey(privKey.readAll(), privateKeyAlgorithm());
        if (d->m_privateKey.isNull()) {
            qCWarning(KDECONNECT_CORE) << "Private key from" << keyPath << "is not valid!";
        }
    }
    return d->m_privateKey.isNull();
}

bool KdeConnectConfig::loadCertificate(const QString &certPath)
{
    QFile cert(certPath);
    QDateTime now = QDateTime::currentDateTime();

    if (cert.exists() && cert.open(QIODevice::ReadOnly)) {
        d->m_certificate = QSslCertificate(cert.readAll());
        if (d->m_certificate.isNull()) {
            qCWarning(KDECONNECT_CORE) << "Certificate from" << certPath << "is not valid";
        } else if (d->m_certificate.effectiveDate() >= now) {
            qCWarning(KDECONNECT_CORE) << "Certificate from" << certPath << "not yet effective: " << d->m_certificate.effectiveDate();
            return true;
        } else if (d->m_certificate.expiryDate() <= now) {
            qCWarning(KDECONNECT_CORE) << "Certificate from" << certPath << "expired: " << d->m_certificate.expiryDate();
            return true;
        }
    }

    return d->m_certificate.isNull();
}

void KdeConnectConfig::loadOrGeneratePrivateKeyAndCertificate(const QString &keyPath, const QString &certPath)
{
    bool needsToGenerateKey = loadPrivateKey(keyPath);
    bool needsToGenerateCert = needsToGenerateKey || loadCertificate(certPath);

    if (needsToGenerateKey) {
        generatePrivateKey(keyPath);
    }
    if (needsToGenerateCert) {
        removeAllTrustedDevices();
        generateCertificate(certPath);
    }

    // Extra security check
    if (QFile::permissions(keyPath) != strictPermissions) {
        qCWarning(KDECONNECT_CORE) << "Warning: KDE Connect private key file has too open permissions " << keyPath;
    }
    if (QFile::permissions(certPath) != strictPermissions) {
        qCWarning(KDECONNECT_CORE) << "Warning: KDE Connect certificate file has too open permissions " << certPath;
    }
}

void KdeConnectConfig::generatePrivateKey(const QString &keyPath)
{
    qCDebug(KDECONNECT_CORE) << "Generating private key";

    d->m_privateKey = SslHelper::generateEcPrivateKey();
    if (d->m_privateKey.isNull()) {
        qCritical(KDECONNECT_CORE) << "Could not generate the private key";
        //Daemon::instance()->reportError(i18n("KDE Connect failed to start"), i18n("Could not generate the private key."));
    }

    QFile privKey(keyPath);
    bool error = false;
    if (!privKey.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        error = true;
    } else {
        privKey.setPermissions(strictPermissions);
        int written = privKey.write(d->m_privateKey.toPem());
        if (written <= 0) {
            error = true;
        }
    }

    d->m_config->setValue(QStringLiteral("keyAlgorithm"), QStringLiteral("EC"));
    d->m_config->sync();

    if (error) {
        qCritical(KDECONNECT_CORE) << "Could not store private key file:" << keyPath;
    }
}

void KdeConnectConfig::generateCertificate(const QString &certPath)
{
    qCDebug(KDECONNECT_CORE) << "Generating certificate";

    QString uuid = QUuid::createUuid().toString(QUuid::Id128).toLower();
    DeviceInfo::filterNonExportableCharacters(uuid);
    qCDebug(KDECONNECT_CORE) << "My id:" << uuid;

    d->m_certificate = SslHelper::generateSelfSignedCertificate(d->m_privateKey, uuid);
    if (d->m_certificate.isNull()) {
        qCritical(KDECONNECT_CORE) << "Could not generate a certificate";
    }

    QFile cert(certPath);
    bool error = false;
    if (!cert.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        error = true;
    } else {
        cert.setPermissions(strictPermissions);
        int written = cert.write(d->m_certificate.toPem());
        if (written <= 0) {
            error = true;
        }
    }

    if (error) {
        qCritical(KDECONNECT_CORE) << "Could not store certificate file:" << certPath;
    }
}
