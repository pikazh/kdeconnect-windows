#include "desktopfile.h"

#include <QLocale>

DesktopFile::DesktopFile(const QString &filePath, QObject *parent)
    : QObject(parent)
    , m_settings(filePath, QSettings::IniFormat)
{}

QString DesktopFile::readIcon()
{
    m_settings.beginGroup(QLatin1StringView("Desktop Entry"));
    QString icon = qvariant_cast<QString>(m_settings.value(QLatin1StringView("Icon")));
    m_settings.endGroup();
    return icon;
}

QString DesktopFile::readName()
{
    m_settings.beginGroup(QLatin1StringView("Desktop Entry"));
    QString name = translatedValue(QLatin1StringView("Name"));
    m_settings.endGroup();
    return name;
}

QString DesktopFile::translatedValue(QAnyStringView key, const QString &defValue)
{
    QString languageWithTerritory = QLocale().name();

    QString trKey = key.toString() + QLatin1Char('[') + languageWithTerritory + QLatin1Char(']');
    if (m_settings.contains(trKey)) {
        return qvariant_cast<QString>(m_settings.value(trKey));
    }

    QString language = languageWithTerritory.sliced(0,
                                                    languageWithTerritory.indexOf(QLatin1Char('_')));
    trKey = key.toString() + QLatin1Char('[') + language + QLatin1Char(']');
    if (m_settings.contains(trKey)) {
        return qvariant_cast<QString>(m_settings.value(trKey));
    }

    return qvariant_cast<QString>(m_settings.value(key, defValue));
}
