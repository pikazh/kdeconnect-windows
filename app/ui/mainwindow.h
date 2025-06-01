#pragma once

#include <QMainWindow>

#include "devicelistmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow() override;

protected Q_SLOTS:
    void on_appSettingsButton_clicked();
    void on_pairButton_clicked();
    void on_openDevicePageButton_clicked();
    void on_refreshDeviceListButton_clicked();
    void on_smsButton_clicked();

    // slots for signals from device list(ui->deviceList)
    void onDeviceListCurrentRowChanged(const QModelIndex &current);
    void on_deviceList_activated(const QModelIndex &index);

    // slots for signals from DeviceManager
    void onDeviceStatusChanged(const QString &devId);
    void onDeviceRemoved(const QString &devId);

protected:
    Device::Ptr deviceObjFromDeviceList(const QModelIndex &index);
    void changeButtonStateForDevice(Device::Ptr dev);

    void retranslateUi();

    virtual void closeEvent(QCloseEvent *) override;
    virtual void changeEvent(QEvent *e) override;

Q_SIGNALS:
    void aboutToClose();

private:
    Ui::MainWindow *ui;

    DeviceListModel *m_deviceListModel;

    Device::Ptr m_currentSelectedDevice;
};
