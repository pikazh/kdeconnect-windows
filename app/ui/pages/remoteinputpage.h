#pragma once

#include "device.h"
#include "plugin/remotekeyboardpluginwrapper.h"
#include "plugin/remotemousepadpluginwrapper.h"
#include "ui/pages/BasePage.h"

#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QWidget>

namespace Ui {
class RemoteInputPage;
}

class RemoteInputPage : public QWidget, public BasePage
{
    Q_OBJECT
public:
    explicit RemoteInputPage(Device::Ptr device, QWidget *parent = nullptr);
    virtual ~RemoteInputPage() override;

    virtual QString displayName() const override { return tr("Remote input"); }
    virtual QIcon icon() const override;
    virtual QString id() const override { return "RemoteInput"; }
    virtual bool apply() override;
    virtual QString helpPage() const override { return ""; }
    virtual bool shouldDisplay() const override;
    virtual void retranslate() override;

protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

    virtual bool eventFilter(QObject *obj, QEvent *ev) override;
    bool handleInputMethodEventForRealTimeInputEdit(QInputMethodEvent *evt);
    bool handleKeyPressedEventForRealTimeInputEdit(QKeyEvent *evt);

    void uiSetKeyboardControlsVisible(bool visible);
    void uiSetRemoteMouseTipsState();

protected Q_SLOTS:
    void lockMouse(bool lock);

    void onPluginRemoteStateChanged();

    void on_lockMouseButton_clicked();

private:
    Ui::RemoteInputPage *ui;
    RemoteKeyboardPluginWrapper *m_remoteKBPluginWrapper;
    RemoteMousePadPluginWrapper *m_remoteMousePadPluginWrapper;
    bool m_mouseLocked = false;
    QPoint m_originalMousePos;
};
