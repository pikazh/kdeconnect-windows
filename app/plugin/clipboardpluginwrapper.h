#pragma once

#include "pluginwrapperbase.h"

class ClipboardPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    ClipboardPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~ClipboardPluginWrapper() override = default;

public Q_SLOTS:
    void sendClipboard();
};
