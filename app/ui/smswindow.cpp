#include "smswindow.h"
#include "ui/smsconversationswidget.h"
#include "ui_smswindow.h"

#include "device.h"
#include "devicemanager.h"

#include "app_debug.h"

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>

SmsWindow::SmsWindow(DeviceManager *deviceManager, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SMSWindow)
    , m_deviceManager(deviceManager)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    ui->conversationStacks->addWidget(new SmsConversationsWidget(Device::Ptr{}, this));
    ui->conversationStacks->setCurrentIndex(0);

    createMenuBar();
}

SmsWindow::~SmsWindow()
{
    delete ui;
}

void SmsWindow::closeEvent(QCloseEvent *evt)
{
    emit aboutToClose();
    evt->accept();
}

void SmsWindow::createMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    QMenu *menu = menuBar->addMenu(tr("Operations"));
    m_deviceMenu = menu->addMenu(tr("Devices"));
    QAction *refreshAction = menu->addAction(tr("Refresh"));
    QObject::connect(refreshAction,
                     &QAction::triggered,
                     this,
                     &SmsWindow::refreshMessages,
                     Qt::QueuedConnection);

    auto deviceList = m_deviceManager->devicesList();
    deviceList.removeIf([](Device::Ptr dev) { return !(dev->isReachable() && dev->isPaired()); });

    // use m_deviceNameToMenuActionsAndIds to sort devices by device name
    for (auto it = deviceList.begin(); it != deviceList.end(); ++it) {
        auto &dev = (*it);
        m_deviceNameToMenuActionsAndIds.insert(dev->name(), {nullptr, dev->id()});
    }

    m_actionGroup = new QActionGroup(m_deviceMenu);
    m_actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

    for (auto it = m_deviceNameToMenuActionsAndIds.begin();
         it != m_deviceNameToMenuActionsAndIds.end();
         ++it) {
        QAction *action = m_deviceMenu->addAction(it.key());
        action->setCheckable(true);
        m_actionGroup->addAction(action);
        auto &pair = it.value();
        pair.first = action;
        action->setProperty("deviceID", pair.second);
        QObject::connect(
            action,
            &QAction::triggered,
            this,
            [this](bool checked) {
                if (checked) {
                    auto action = QObject::sender();
                    QString deviceID = qvariant_cast<QString>(action->property("deviceID"));
                    if (!deviceID.isEmpty()) {
                        selectDevice(deviceID);
                    }
                }
            },
            Qt::QueuedConnection);
    }

    QObject::connect(m_deviceManager,
                     &DeviceManager::deviceAdded,
                     this,
                     &SmsWindow::onDeviceAdd,
                     Qt::UniqueConnection);

    QObject::connect(m_deviceManager,
                     &DeviceManager::deviceRemoved,
                     this,
                     &SmsWindow::onDeviceRemoved,
                     Qt::UniqueConnection);

    QObject::connect(m_deviceManager,
                     &DeviceManager::deviceVisibilityChanged,
                     this,
                     &SmsWindow::onDeviceVisibilityChanged,
                     Qt::UniqueConnection);

    QObject::connect(m_deviceManager,
                     &DeviceManager::devicePairStateChanged,
                     this,
                     &SmsWindow::onDevicePairStateChanged,
                     Qt::UniqueConnection);
}

void SmsWindow::addOrRemoveDeviceEntryFromMenu(const QString &deviceId)
{
    auto dev = m_deviceManager->getDevice(deviceId);

    bool found = false;
    auto it = m_deviceNameToMenuActionsAndIds.begin();
    for (; it != m_deviceNameToMenuActionsAndIds.end(); ++it) {
        if (it.value().second == deviceId) {
            found = true;
            break;
        }
    }

    if (dev && dev->isPaired() && dev->isReachable()) {
        Q_ASSERT(!found);
        if (!found) {
            QAction *action = new QAction(dev->name(), m_deviceMenu);
            auto insertedIt = m_deviceNameToMenuActionsAndIds.insert(dev->name(),
                                                                     {action, deviceId});
            insertedIt++;
            if (insertedIt != m_deviceNameToMenuActionsAndIds.end()) {
                m_deviceMenu->insertAction(insertedIt.value().first, action);
            } else {
                m_deviceMenu->insertAction(nullptr, action);
            }
            action->setCheckable(true);
            m_actionGroup->addAction(action);
            action->setProperty("deviceID", deviceId);
            QObject::connect(
                action,
                &QAction::triggered,
                this,
                [this](bool checked) {
                    if (checked) {
                        auto action = QObject::sender();
                        QString deviceID = qvariant_cast<QString>(action->property("deviceID"));
                        if (!deviceID.isEmpty()) {
                            this->selectDevice(deviceID);
                        }
                    }
                },
                Qt::QueuedConnection);

            qDebug(KDECONNECT_APP) << "add device item to menu. deviceID:" << deviceId
                                   << ", deviceName:" << dev->name();
        }
    } else {
        if (found) {
            m_deviceMenu->removeAction(it.value().first);
            m_actionGroup->removeAction(it.value().first);
            it.value().first->deleteLater();
            m_deviceNameToMenuActionsAndIds.erase(it);

            qDebug(KDECONNECT_APP) << "remove device item from menu. deviceID:" << deviceId
                                   << ", deviceName:" << dev->name();
        }
    }
}

void SmsWindow::refreshMessages() {}

void SmsWindow::selectDevice(const QString &deviceId)
{
    if (m_selectedDeviceId != deviceId) {
        qDebug(KDECONNECT_APP) << "device" << deviceId << "selected";
        m_selectedDeviceId = deviceId;
        auto dev = m_deviceManager->getDevice(m_selectedDeviceId);
        if (dev) {
            auto it = m_DeviceIdToConversationsWidget.find(m_selectedDeviceId);
            if (it != m_DeviceIdToConversationsWidget.end()) {
                ui->conversationStacks->setCurrentWidget(it.value());
            } else {
                auto smsWidget = new SmsConversationsWidget(dev, this);
                ui->conversationStacks->addWidget(smsWidget);
                ui->conversationStacks->setCurrentWidget(smsWidget);
                m_DeviceIdToConversationsWidget.insert(deviceId, smsWidget);
            }
            //
        }
    }
}

void SmsWindow::onDeviceAdd(const QString &deviceID)
{
    addOrRemoveDeviceEntryFromMenu(deviceID);
}

void SmsWindow::onDeviceRemoved(const QString &deviceID)
{
    addOrRemoveDeviceEntryFromMenu(deviceID);

    auto it = m_DeviceIdToConversationsWidget.find(deviceID);
    if (it != m_DeviceIdToConversationsWidget.end()) {
        ui->conversationStacks->removeWidget(it.value());
        it.value()->deleteLater();
        m_DeviceIdToConversationsWidget.erase(it);
        ui->conversationStacks->setCurrentIndex(0);
    }
}

void SmsWindow::onDeviceVisibilityChanged(const QString &deviceID, bool isVisible)
{
    addOrRemoveDeviceEntryFromMenu(deviceID);
}

void SmsWindow::onDevicePairStateChanged(const QString &deviceID, Device::PairState state)
{
    addOrRemoveDeviceEntryFromMenu(deviceID);
}
