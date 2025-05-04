#pragma once

#include "kdeconnectcore_export.h"

#include <QJsonObject>
#include <QString>

class KDECONNECTCORE_EXPORT JsonUtils
{
public:
    static QJsonValue readTranslatedValue(const QJsonObject &obj, const QString &key, const QJsonValue defaultValue = QJsonValue());

};
