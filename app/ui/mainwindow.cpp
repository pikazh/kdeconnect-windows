#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "application.h"
#include "devicemanager.h"

#include "app_debug.h"

#include "core/kdeconnectconfig.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QString>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    ui->refreshDeviceListButton->setIcon(RETRIEVE_THEME_ICON("view-refresh"));
    ui->pairButton->setIcon(RETRIEVE_THEME_ICON("network-connect"));
    ui->pairButton->setDisabled(true);
    ui->openDevicePageButton->setIcon(RETRIEVE_THEME_ICON("smartphonedisconnected"));
    ui->openDevicePageButton->setDisabled(true);
    ui->appSettingsButton->setIcon(RETRIEVE_THEME_ICON("configure"));
    ui->smsButton->setIcon(RETRIEVE_THEME_ICON("mail-message-new-list"));

    m_deviceListModel = new DeviceListModel(this);
    DeviceListProxyModel *proxyModel = new DeviceListProxyModel(this);
    proxyModel->setSourceModel(m_deviceListModel);
    ui->deviceList->setModel(proxyModel);

    ui->deviceList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->deviceList->setAlternatingRowColors(false);
    ui->deviceList->setRootIsDecorated(false);
    ui->deviceList->setItemsExpandable(false);
    ui->deviceList->header()->setCascadingSectionResizes(true);
    ui->deviceList->header()->setStretchLastSection(true);
    ui->deviceList->setColumnWidth(0, 200);
    ui->deviceList->sortByColumn(2, Qt::AscendingOrder);
    QObject::connect(ui->deviceList->selectionModel(),
                     SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                     this,
                     SLOT(onDeviceListCurrentRowChanged(QModelIndex)));

    auto deviceManager = APPLICATION->deviceManager();
    m_deviceListModel->setDeviceList(deviceManager->devicesList());
    QObject::connect(deviceManager, &DeviceManager::deviceListChanged, this, [this, deviceManager]() {
        m_deviceListModel->setDeviceList(deviceManager->devicesList());
    });

    auto kdeConnectConfig = KdeConnectConfig::instance();
    auto localDeviceName = kdeConnectConfig->name();
    ui->deviceNameLabel->setText(localDeviceName);

    QObject::connect(
        kdeConnectConfig,
        &KdeConnectConfig::deviceNameChanged,
        this,
        [this](const QString &deviceName) { ui->deviceNameLabel->setText(deviceName); },
        Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_appSettingsButton_clicked()
{
    APPLICATION->showAppSettingsDialog();
}

void MainWindow::on_pairButton_clicked()
{
    if (!ui->deviceList->currentIndex().isValid()) {
        return;
    }

    auto device = deviceObjFromDeviceList(ui->deviceList->currentIndex());
    if (!device->isReachable()) {
        if (device->isPaired()) {
            device->unpair();
        }
    } else {
        if (!device->isPaired()) {
            APPLICATION->showDeviceWindow(device);
        } else {
            // todo: add some alert
            device->unpair();
        }
    }
}

void MainWindow::on_openDevicePageButton_clicked()
{
    if (!ui->deviceList->currentIndex().isValid()) {
        return;
    }

    auto device = deviceObjFromDeviceList(ui->deviceList->currentIndex());
    if (device->isReachable()) {
        APPLICATION->showDeviceWindow(device);
    }
}

void MainWindow::on_refreshDeviceListButton_clicked()
{
    auto deviceManager = APPLICATION->deviceManager();
    QMetaObject::invokeMethod(deviceManager,
                              &DeviceManager::refreshNetwokState,
                              Qt::QueuedConnection);
}

void MainWindow::on_smsButton_clicked()
{
    APPLICATION->showSmsConversationsWindow();
}

void MainWindow::onDeviceListCurrentRowChanged(const QModelIndex &current)
{
    if (!current.isValid()) {
        return;
    }

    auto device = deviceObjFromDeviceList(current);
    qDebug(KDECONNECT_APP) << "current select device:" << device->name();
    changeButtonStateForDevice(device);
}

void MainWindow::on_deviceList_activated(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    auto device = deviceObjFromDeviceList(index);
    if (device->isReachable()) {
        APPLICATION->showDeviceWindow(device);
    }
}

Device::Ptr MainWindow::deviceObjFromDeviceList(const QModelIndex &index)
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
        } else {
            ui->pairButton->setText(tr("Pair"));
            ui->pairButton->setIcon(RETRIEVE_THEME_ICON("network-connect"));
        }
    } else if (dev->isPaired()) {
        ui->pairButton->setEnabled(true);
        ui->pairButton->setText(tr("Unpair"));
        ui->pairButton->setIcon(RETRIEVE_THEME_ICON("network-disconnect"));
    } else {
        ui->pairButton->setEnabled(false);
    }

    ui->openDevicePageButton->setEnabled(dev->isReachable());
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    emit aboutToClose();
    evt->accept();
}
