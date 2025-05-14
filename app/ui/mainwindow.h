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
    virtual ~MainWindow() override;

protected Q_SLOTS:
    void on_modifyDeviceNameButton_clicked();
    void on_pairButton_clicked();
    void on_openDevicePageButton_clicked();
    void on_refreshDeviceListButton_clicked();
    void onDeviceListCurrentRowChanged(const QModelIndex &current);
    void on_deviceList_activated(const QModelIndex &index);

protected:
    Device::Ptr deviceObjFromDeviceList(const QModelIndex &index);
    void changeButtonStateForDevice(Device::Ptr dev);

    virtual void closeEvent(QCloseEvent *) override;

Q_SIGNALS:
    void aboutToClose();

private:
    Ui::MainWindow *ui;

    DeviceListModel *m_deviceListModel;
};
#endif // MAINWINDOW_H
