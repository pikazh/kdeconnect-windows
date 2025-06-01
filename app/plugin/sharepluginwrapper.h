#pragma once

#include "pluginwrapperbase.h"

#include <QString>
#include <QUrl>

class SharePluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit SharePluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);

public Q_SLOTS:
    void shareText(const QString &text);
    void shareUrl(const QUrl &url);
};
