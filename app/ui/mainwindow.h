#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "devicelistmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected Q_SLOTS:
    void on_modifyDeviceNameButton_clicked();
    void on_pairButton_clicked();
    void on_pingButton_clicked();
    void on_pluginConfigButton_clicked();
    void on_refreshDeviceListButton_clicked();
    void on_deviceList_activated(const QModelIndex &index);
    void on_deviceList_pressed(const QModelIndex &index);

protected:
    Device::Ptr retrieveDeviceObjFromDeviceList(const QModelIndex &index);
    void changeButtonStateForDevice(Device::Ptr dev);

private:
    Ui::MainWindow *ui;

    DeviceListModel *m_deviceListModel;
};
#endif // MAINWINDOW_H
