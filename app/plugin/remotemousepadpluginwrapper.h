#pragma once

#include "pluginwrapperbase.h"

class RemoteMousePadPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit RemoteMousePadPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~RemoteMousePadPluginWrapper() override = default;

public Q_SLOTS:
    void moveCursor(const QPoint &p);

    void sendSingleClick();
    void sendMiddleClick();
    void sendRightClick();
    void sendSingleHold();
    void sendSingleRelease();
    void sendScroll(int dx, int dy);
};
