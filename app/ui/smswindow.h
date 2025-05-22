#pragma once

#include "devicemanager.h"

#include <QActionGroup>
#include <QHash>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMultiMap>
#include <QString>

#include <utility>

namespace Ui {
class SMSWindow;
}

class SmsConversationsWidget;

class SmsWindow : public QMainWindow
{
    Q_OBJECT
public:
    SmsWindow(DeviceManager *deviceManager, QWidget *parent = nullptr);
    virtual ~SmsWindow() override;

protected:
    virtual void closeEvent(QCloseEvent *) override;

    void createMenuBar();
    void addOrRemoveDeviceEntryFromMenu(const QString &deviceId);

protected Q_SLOTS:
    void refreshMessages();
    void selectDevice(const QString &deviceId);

    void onDeviceAdd(const QString &deviceID);
    void onDeviceRemoved(const QString &deviceID);
    void onDeviceVisibilityChanged(const QString &deviceID, bool isVisible);
    void onDevicePairStateChanged(const QString &deviceID, Device::PairState state);

Q_SIGNALS:
    void aboutToClose();

private:
    Ui::SMSWindow *ui;
    QMenu *m_deviceMenu = nullptr;
    QActionGroup *m_actionGroup = nullptr;
    DeviceManager *m_deviceManager = nullptr;
    QMultiMap<QString, std::pair<QAction *, QString>> m_deviceNameToMenuActionsAndIds;

    QString m_selectedDeviceId;
    QHash<QString, SmsConversationsWidget *> m_DeviceIdToConversationsWidget;
};
