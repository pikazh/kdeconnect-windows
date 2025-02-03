#pragma once

#include "coreaddons_export.h"

#include <QJsonObject>
#include <QString>

class COREADDONS_EXPORT JsonUtils
{
public:
    static QJsonValue readTranslatedValue(const QJsonObject &obj, const QString &key, const QJsonValue defaultValue = QJsonValue());

};
