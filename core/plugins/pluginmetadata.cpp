#include "pluginmetadata.h"
#include "core_debug.h"
#include "jsonutils.h"

#include <QSharedData>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDirIterator>
#include <QLibrary>
#include <QPluginLoader>

class PluginMetaDataPrivate : public QSharedData
{
public:
    PluginMetaDataPrivate(const QJsonObject &obj, const QString &filePath)
        : m_metaData(obj)
        , m_rootObject(obj.value(QLatin1StringView("KPlugin")).toObject())
        , m_filePath(filePath)
        , m_fileName(QFileInfo(filePath).completeBaseName())
    {

    }

    static void forEachPlugin(std::function<void(const QFileInfo &)> callback)
    {
        const QString pluginsDir = QCoreApplication::applicationDirPath() + QDir::separator() + QLatin1StringView("kdeconnect_plugins");
        QDirIterator it(pluginsDir, QDir::Files);

        while (it.hasNext()) {
            it.next();

            if (QLibrary::isLibrary(it.fileName())) {
                callback(it.fileInfo());
            }
        }
    }

    static PluginMetaDataPrivate* ofPath(const QString &path)
    {
        QPluginLoader loader;
        loader.setFileName(path);
        const QJsonObject metaData = loader.metaData();
        QFileInfo fileInfo(path);
        auto ret = new PluginMetaDataPrivate(metaData.value(QLatin1StringView("MetaData")).toObject(), fileInfo.absoluteFilePath());
        return ret;
    }

    const QJsonObject m_metaData;
    const QJsonObject m_rootObject;
    const QString m_filePath;
    const QString m_fileName;
};

PluginMetaData::PluginMetaData()
    :d(new PluginMetaDataPrivate(QJsonObject(), QString()))
{

}

PluginMetaData::PluginMetaData(const QString &pluginFilePath)
    :d(PluginMetaDataPrivate::ofPath(pluginFilePath))
{
}

PluginMetaData::PluginMetaData(const PluginMetaData &other)
    :d(other.d)
{

}

PluginMetaData &PluginMetaData::operator=(const PluginMetaData &other)
{
    d = other.d;
    return *this;
}

PluginMetaData::~PluginMetaData()
{

}

QList<PluginMetaData> PluginMetaData::load()
{
    QList<PluginMetaData> ret;
    QSet<QString> addedPlugins;
    PluginMetaDataPrivate::forEachPlugin([&](const QFileInfo &fileInfo){
        QString filePath = fileInfo.absoluteFilePath();
        auto metaData = PluginMetaData(filePath);
        if(!metaData.isValid())
        {
            qWarning(KDECONNECT_CORE) << filePath << " doesn't contain valid JSON metadata.";
            return;
        }

        if (addedPlugins.contains(metaData.name())) {
            return;
        }

        addedPlugins << metaData.name();
        ret.append(metaData);
    });

    return ret;
}

bool PluginMetaData::isValid() const
{
    if(rawData().isEmpty())
        return false;

    if (id().isEmpty())
        return false;

    return true;
}

QJsonObject PluginMetaData::rawData() const
{
    return d->m_metaData;
}

QString PluginMetaData::filePath() const
{
    return d->m_filePath;
}

QString PluginMetaData::name() const
{
    return JsonUtils::readTranslatedValue(d->m_rootObject, QStringLiteral("Name")).toString();
}

QString PluginMetaData::id() const
{
    return d->m_rootObject[QLatin1StringView("Name")].toString();
}

QString PluginMetaData::description() const
{
    return JsonUtils::readTranslatedValue(d->m_rootObject, QStringLiteral("Description")).toString();
}

QString PluginMetaData::iconName() const
{
    return d->m_rootObject[QLatin1StringView("Icon")].toString();
}

QString PluginMetaData::fileName() const
{
    return d->m_fileName;
}

QString PluginMetaData::version() const
{
    return d->m_rootObject[QLatin1StringView("Version")].toString();
}

bool PluginMetaData::isEnabledByDefault() const
{
    const QLatin1StringView key("EnabledByDefault");
    const QJsonValue val = d->m_rootObject[key];
    if (val.isBool()) {
        return val.toBool();
    } else if (val.isString()) {
        qWarning(KDECONNECT_CORE) << "Expected JSON property " << key << " in " << d->m_filePath
                                  << " to be boolean, but it was a string";
        return val.toString() == QLatin1StringView("true");
    }
    return false;
}

QString PluginMetaData::value(QStringView key, const QString &defaultValue) const
{
    const QJsonValue val = rawData().value(key);
    if(val.isString())
    {
        return val.toString();
    }
    else if(val.isArray())
    {
        qWarning(KDECONNECT_CORE) << "Expected JSON property " << key << " in " << d->m_filePath
                                  << " to be a single string, but it is an array";
        return val.toVariant().toStringList().join(',');
    }
    else if (val.isBool()) {
        qWarning(KDECONNECT_CORE) << "Expected JSON property " << key << " in " << d->m_filePath
                                  << " to be a single string, but it is a bool";
        return val.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }
    return defaultValue;
}

QStringList PluginMetaData::value(QStringView key, const QStringList &defaultValue) const
{
    const QJsonValue value = rawData().value(key);
    if (value.isUndefined() || value.isNull())
    {
        return defaultValue;
    }
    else if (value.isObject()) {
        qWarning(KDECONNECT_CORE) << "Expected JSON property " << key
                                  << " to be a string list, instead an object was specified in "
                                  << d->m_filePath;
        return defaultValue;
    }
    else if (value.isArray())
    {
        return value.toVariant().toStringList();
    }
    else
    {
        const QString asString = value.isString() ? value.toString() : value.toVariant().toString();
        if (asString.isEmpty())
        {
            return defaultValue;
        }
        qWarning(KDECONNECT_CORE) << "Expected JSON property " << key << "to be a string list in "
                                  << d->m_filePath
                                  << " Treating it as a list with a single entry: " << asString;
        return QStringList(asString);
    }
}

int PluginMetaData::value(QStringView key, int defaultValue) const
{
    const QJsonValue value = rawData().value(key);
    if (value.isDouble())
    {
        return value.toInt();
    }
    else if (value.isString())
    {
        const QString intString = value.toString();
        bool ok = false;
        int convertedIntValue = intString.toInt(&ok);
        if (ok)
        {
            return convertedIntValue;
        }
        else
        {
            qWarning(KDECONNECT_CORE) << "Expected " << key << " to be an int, instead "
                                      << intString << " was specified in " << d->m_filePath;
            return defaultValue;
        }
    }
    else
    {
        return defaultValue;
    }
}

bool PluginMetaData::value(QStringView key, bool defaultValue) const
{
    const QJsonValue value = rawData().value(key);
    if (value.isBool())
    {
        return value.toBool();
    }
    else if (value.isString())
    {
        return value.toString() == QLatin1StringView("true");
    }
    else
    {
        return defaultValue;
    }
}
