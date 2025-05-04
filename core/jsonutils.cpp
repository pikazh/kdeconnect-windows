#include "jsonutils.h"
#include <QLocale>
#include <QStringView>

QJsonValue JsonUtils::readTranslatedValue(const QJsonObject &obj, const QString &key, const QJsonValue defaultValue)
{
    QString languageWithTerritory = QLocale().name();
    auto it = obj.constFind(key + QLatin1Char('[') + languageWithTerritory + QLatin1Char(']'));
    if(it != obj.constEnd())
    {
        return it.value();
    }

    QString language = languageWithTerritory.sliced(0, languageWithTerritory.indexOf(QLatin1Char('_')));
    it = obj.constFind(key + QLatin1Char('[') + language + QLatin1Char(']'));
    if(it != obj.constEnd())
    {
        return it.value();
    }

    it = obj.constFind(key);
    if(it != obj.constEnd())
    {
        return it.value();
    }

    return defaultValue;
}
