#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "app_debug.h"
#include "application.h"
#include "devicemanager.h"
#include "kdeconnectconfig.h"

#include <QInputDialog>
#include <QString>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->refreshDeviceListButton->setIcon(RETRIEVE_THEME_ICON("view-refresh"));
    ui->pairButton->setIcon(RETRIEVE_THEME_ICON("network-connect"));
    ui->pingButton->setIcon(RETRIEVE_THEME_ICON("hands-free"));
    ui->pluginConfigButton->setIcon(RETRIEVE_THEME_ICON("configure"));
    ui->modifyDeviceNameButton->setIcon(RETRIEVE_THEME_ICON("configure"));

    ui->pairButton->setDisabled(true);
    ui->pingButton->setDisabled(true);
    ui->pluginConfigButton->setDisabled(true);

    m_deviceListModel = new DeviceListModel(this);
    DeviceListProxyModel *proxyModel = new DeviceListProxyModel(this);
    proxyModel->setSourceModel(m_deviceListModel);
    ui->deviceList->setModel(proxyModel);

    ui->deviceList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->deviceList->setAlternatingRowColors(true);
    ui->deviceList->setRootIsDecorated(false);
    ui->deviceList->setItemsExpandable(false);
    ui->deviceList->header()->setCascadingSectionResizes(true);
    ui->deviceList->header()->setStretchLastSection(true);
    ui->deviceList->setColumnWidth(0, 200);
    ui->deviceList->sortByColumn(2, Qt::AscendingOrder);

    auto deviceManager = APPLICATION->deviceManager();
    m_deviceListModel->setDeviceList(deviceManager->devicesList());
    QObject::connect(deviceManager, &DeviceManager::deviceListChanged, this, [this, deviceManager]() {
        m_deviceListModel->setDeviceList(deviceManager->devicesList());
    });

    auto localDeviceName = KdeConnectConfig::instance().name();
    ui->deviceNameLabel->setText(localDeviceName);

    //QMetaObject::connectSlotsByName(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_modifyDeviceNameButton_clicked()
{
    auto localDeviceName = KdeConnectConfig::instance().name();
    bool ok = false;
    QString text = QInputDialog::getText(this,
                                         tr("Please input the device name"),
                                         tr("Device Name: "),
                                         QLineEdit::Normal,
                                         localDeviceName,
                                         &ok);
    if (ok && text != localDeviceName) {
        QString filteredName = DeviceInfo::filterName(text);
        qCDebug(KDECONNECT_APP) << "New device name" << filteredName;
        KdeConnectConfig::instance().setName(filteredName);
        ui->deviceNameLabel->setText(filteredName);
        //forceOnNetworkChange();
    }
}

void MainWindow::on_pairButton_clicked() {}

void MainWindow::on_pingButton_clicked() {}

void MainWindow::on_pluginConfigButton_clicked() {}

void MainWindow::on_refreshDeviceListButton_clicked() {}

void MainWindow::on_deviceList_activated(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    auto device = retrieveDeviceObjFromDeviceList(index);
    APPLICATION->showDeviceWindow(device);
}

void MainWindow::on_deviceList_pressed(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    auto device = retrieveDeviceObjFromDeviceList(index);
    changeButtonStateForDevice(device);
}

Device::Ptr MainWindow::retrieveDeviceObjFromDeviceList(const QModelIndex &index)
{
    DeviceListProxyModel *deviceListProxyModel = qobject_cast<DeviceListProxyModel *>(
        ui->deviceList->model());
    Q_ASSERT(deviceListProxyModel != nullptr);
    auto sourceIndex = deviceListProxyModel->mapToSource(index);
    auto device = m_deviceListModel->at(sourceIndex.row());
    return device;
}

void MainWindow::changeButtonStateForDevice(Device::Ptr dev)
{
    if (dev->isReachable()) {
        ui->pairButton->setEnabled(true);
        if (dev->isPaired()) {
            ui->pairButton->setText(tr("Unpair"));
            ui->pairButton->setIcon(RETRIEVE_THEME_ICON("network-disconnect"));
            ui->pingButton->setEnabled(true);
            ui->pluginConfigButton->setEnabled(true);
        } else {
            ui->pairButton->setText(tr("Pair"));
            ui->pairButton->setIcon(RETRIEVE_THEME_ICON("network-connect"));
            ui->pingButton->setEnabled(false);
            ui->pluginConfigButton->setEnabled(false);
        }
    } else if (dev->isPaired()) {
        ui->pairButton->setEnabled(true);
        ui->pairButton->setText(tr("Unpair"));
        ui->pairButton->setIcon(RETRIEVE_THEME_ICON("network-disconnect"));
        ui->pingButton->setEnabled(false);
        ui->pluginConfigButton->setEnabled(false);
    } else {
        ui->pairButton->setEnabled(false);
        ui->pingButton->setEnabled(false);
        ui->pluginConfigButton->setEnabled(false);
    }
}
