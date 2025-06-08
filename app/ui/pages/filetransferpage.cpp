#include "filetransferpage.h"
#include "app_debug.h"
#include "ui/uicommon.h"
#include "ui_filetransferpage.h"

#include "core/task/peerfiledownloadtask.h"
#include "core/task/peerfileuploadtask.h"

#include <QAction>
#include <QDesktopServices>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QTableWidgetItem>
#include <QWidget>

constexpr auto operator""_KB(unsigned long long const x) -> qint64
{
    return 1024L * x;
}

constexpr auto operator""_MB(unsigned long long const x) -> qint64
{
    return 1024L * 1024L * x;
}

constexpr auto operator""_GB(unsigned long long const x) -> qint64
{
    return 1024L * 1024L * 1024L * x;
}

QString trimDecimals(QString src, int cnt)
{
    auto index = src.indexOf(QLatin1Char('.'));
    if (index != -1) {
        bool foundNotZero = false;
        for (; cnt-- > 0 && ++index < src.length();) {
            if (src.at(index) != QLatin1Char('0')) {
                foundNotZero = true;
                break;
            }
        }

        if (!foundNotZero) {
            src = src.left(index);
        } else {
            if (index + 1 + cnt < src.length())
                src = src.left(index + 1 + cnt);
        }
    }

    return src;
}

FileTransferPage::FileTransferPage(Device::Ptr device, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileTransferPage)
    , m_sharePluginWrapper(new SharePluginWrapper(device, this))
    , m_transferHistoryListModel(new TransferHistoryListModel(this))
{
    ui->setupUi(this);
    ui->transferingList->setColumnCount(Columns::Count);
    QStringList headerNames;
    headerNames.push_back(tr("Operation"));
    headerNames.push_back(tr("File"));
    headerNames.push_back(tr("Transferred"));
    headerNames.push_back(tr("Progress"));
    headerNames.push_back(tr("Cancel"));
    ui->transferingList->setHorizontalHeaderLabels(headerNames);

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(m_transferHistoryListModel);
    proxyModel->setSortRole(TransferHistoryListItemDataRoles::FinishTime);
    proxyModel->sort(TransferHistoryListModel::Column::FinishTime, Qt::DescendingOrder);
    ui->historyList->setModel(proxyModel);
    ui->historyList->setContextMenuPolicy(Qt::CustomContextMenu);

    QObject::connect(ui->historyList,
                     &QWidget::customContextMenuRequested,
                     this,
                     &FileTransferPage::createHistoryListMenu);

    QObject::connect(ui->historyList,
                     &QTreeView::activated,
                     this,
                     &FileTransferPage::onHistoryItemActivated);

    auto initFunctor = [this]() {
        m_recvFilesTaskSchedule = m_sharePluginWrapper->recvFilesTaskSchedule();
        m_sendFilesTaskSchedule = m_sharePluginWrapper->sendFilesTaskSchedule();
        m_transferHistoryManager = m_sharePluginWrapper->transferHistoryManager();
        m_transferHistoryManager->registerHistoryChangeListener(m_transferHistoryListModel);
        QObject::connect(m_recvFilesTaskSchedule.get(),
                         &TaskScheduler::taskAdded,
                         this,
                         [this](Task::Ptr task) {
                             PeerFileDownloadTask *fileDlTask = qobject_cast<PeerFileDownloadTask *>(
                                 task.get());
                             Q_ASSERT(fileDlTask != nullptr);
                             if (fileDlTask != nullptr) {
                                 addTransferingListItem(fileDlTask);
                             }
                         });

        QObject::connect(m_recvFilesTaskSchedule.get(),
                         &TaskScheduler::taskRemoved,
                         this,
                         [this](Task::Ptr task) {
                             int rowIndex = findListRowIndexByTask(task.get());
                             Q_ASSERT(rowIndex > -1);
                             if (rowIndex > -1)
                                 ui->transferingList->removeRow(rowIndex);
                         });

        QObject::connect(m_sendFilesTaskSchedule.get(),
                         &TaskScheduler::taskAdded,
                         this,
                         [this](Task::Ptr task) {
                             PeerFileUploadTask *fileUploadTask = qobject_cast<PeerFileUploadTask *>(
                                 task.get());
                             Q_ASSERT(fileUploadTask != nullptr);
                             if (fileUploadTask != nullptr) {
                                 addTransferingListItem(fileUploadTask);
                             }
                         });

        QObject::connect(m_sendFilesTaskSchedule.get(),
                         &TaskScheduler::taskRemoved,
                         this,
                         [this](Task::Ptr task) {
                             int rowIndex = findListRowIndexByTask(task.get());
                             Q_ASSERT(rowIndex > -1);
                             if (rowIndex > -1)
                                 ui->transferingList->removeRow(rowIndex);
                         });

        auto scrollBar = ui->historyList->verticalScrollBar();
        QObject::connect(scrollBar,
                         &QScrollBar::valueChanged,
                         this,
                         &FileTransferPage::onHistoryListVerticalScrollbarValueChanged,
                         Qt::UniqueConnection);

        initTransferingList();
        loadTransferHistoryList();
    };

    QObject::connect(m_sharePluginWrapper,
                     &SharePluginWrapper::pluginLoadedChange,
                     this,
                     [this, initFunctor](bool loaded) {
                         if (loaded) {
                             initFunctor();
                         } else {
                             deleteAllRowsFromTransferingList();
                             clearTransferHistoryList();
                             m_recvFilesTaskSchedule.reset();
                             m_sendFilesTaskSchedule.reset();
                             m_transferHistoryManager.reset();
                         }
                     });

    m_sharePluginWrapper->init();

    initFunctor();
}

FileTransferPage::~FileTransferPage()
{
    m_transferHistoryManager->unRegisterHistoryChangeListener(m_transferHistoryListModel);
    delete ui;
}

QIcon FileTransferPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("folder-network"));
}

bool FileTransferPage::apply()
{
    return true;
}

bool FileTransferPage::shouldDisplay() const
{
    return m_sharePluginWrapper->isPluginLoaded();
}

void FileTransferPage::retranslate()
{
    ui->retranslateUi(this);
}

void FileTransferPage::initTransferingList()
{
    QList<Task::Ptr> waitingTaskList = m_recvFilesTaskSchedule->waitingTasks();
    for (auto task : waitingTaskList) {
        PeerFileDownloadTask *fileDlTask = qobject_cast<PeerFileDownloadTask *>(task.get());
        Q_ASSERT(fileDlTask != nullptr);
        if (fileDlTask != nullptr) {
            addTransferingListItem(fileDlTask);
        }
    }

    QList<Task::Ptr> runningTasks = m_recvFilesTaskSchedule->runningTasks();
    for (auto task : runningTasks) {
        PeerFileDownloadTask *fileDlTask = qobject_cast<PeerFileDownloadTask *>(task.get());
        Q_ASSERT(fileDlTask != nullptr);
        if (fileDlTask != nullptr) {
            addTransferingListItem(fileDlTask);
        }
    }

    waitingTaskList = m_sendFilesTaskSchedule->waitingTasks();
    for (auto task : waitingTaskList) {
        PeerFileUploadTask *fileUploadTask = qobject_cast<PeerFileUploadTask *>(task.get());
        Q_ASSERT(fileUploadTask != nullptr);
        if (fileUploadTask != nullptr) {
            addTransferingListItem(fileUploadTask);
        }
    }

    runningTasks = m_recvFilesTaskSchedule->runningTasks();
    for (auto task : runningTasks) {
        PeerFileUploadTask *fileUploadTask = qobject_cast<PeerFileUploadTask *>(task.get());
        Q_ASSERT(fileUploadTask != nullptr);
        if (fileUploadTask != nullptr) {
            addTransferingListItem(fileUploadTask);
        }
    }
}

void FileTransferPage::deleteAllRowsFromTransferingList()
{
    for (int i = ui->transferingList->rowCount() - 1; i >= 0; --i) {
        ui->transferingList->removeRow(i);
    }
}

void FileTransferPage::addTransferingListItem(PeerFileDownloadTask *fileDlTask)
{
    int rowIndex = ui->transferingList->rowCount();
    ui->transferingList->insertRow(rowIndex);

    QTableWidgetItem *opItem = new QTableWidgetItem(tr("Receiving"));
    opItem->setFlags(opItem->flags() & ~Qt::ItemIsEditable);
    opItem->setData(Qt::UserRole, QVariant::fromValue(fileDlTask));
    ui->transferingList->setItem(rowIndex, Columns::Operation, opItem);

    QString dlFilePath = fileDlTask->downloadFilePath();
    QFileInfo fileInfo(dlFilePath);
    QTableWidgetItem *filePathItem = new QTableWidgetItem(fileInfo.fileName());
    filePathItem->setToolTip(dlFilePath);
    filePathItem->setFlags(opItem->flags() & ~Qt::ItemIsEditable);
    ui->transferingList->setItem(rowIndex, Columns::File, filePathItem);

    qint64 totalProgress = fileDlTask->totalProgress();
    qint64 currentProgress = fileDlTask->currentProgress();
    QString transferredStr;
    if (totalProgress > 0) {
        transferredStr = transferredString(currentProgress, totalProgress);
    } else {
        transferredStr = transferredString(0, fileDlTask->contentSize());
    }
    QTableWidgetItem *transferredItem = new QTableWidgetItem(transferredStr);
    transferredItem->setFlags(opItem->flags() & ~Qt::ItemIsEditable);
    ui->transferingList->setItem(rowIndex, Columns::Transferred, transferredItem);
    QObject::connect(fileDlTask, &Task::progress, this, [this, transferredItem, fileDlTask]() {
        qint64 totalProgress = fileDlTask->totalProgress();
        qint64 currentProgress = fileDlTask->currentProgress();
        transferredItem->setText(transferredString(currentProgress, totalProgress));
    });

    QTableWidgetItem *statusItem = new QTableWidgetItem(taskStatusString(fileDlTask->taskStatus()));
    statusItem->setFlags(opItem->flags() & ~Qt::ItemIsEditable);
    ui->transferingList->setItem(rowIndex, Columns::Progress, statusItem);
    QObject::connect(fileDlTask, &Task::statusChanged, this, [this, statusItem, fileDlTask]() {
        statusItem->setText(taskStatusString(fileDlTask->taskStatus()));
    });

    QWidget *w = new QWidget(ui->transferingList);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    QPushButton *removeButton = new QPushButton(w);
    removeButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeButton->setToolTip(tr("Delete"));
    layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout->addWidget(removeButton);
    layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    w->setFixedWidth(40);
    ui->transferingList->setCellWidget(rowIndex, Columns::Cancel, w);
    QObject::connect(removeButton, &QPushButton::clicked, this, [this, fileDlTask]() {
        if (!fileDlTask->isRunning())
            m_recvFilesTaskSchedule->removeTask(fileDlTask->sharedFromThis());
        else
            m_recvFilesTaskSchedule->abortTask(fileDlTask->sharedFromThis());
    });
}

void FileTransferPage::addTransferingListItem(PeerFileUploadTask *fileUploadTask)
{
    int rowIndex = ui->transferingList->rowCount();
    ui->transferingList->insertRow(rowIndex);

    QTableWidgetItem *opItem = new QTableWidgetItem(tr("Sending"));
    opItem->setFlags(opItem->flags() & ~Qt::ItemIsEditable);
    opItem->setData(Qt::UserRole, QVariant::fromValue(fileUploadTask));
    ui->transferingList->setItem(rowIndex, Columns::Operation, opItem);

    QString dlFilePath = fileUploadTask->uploadFilePath();
    QFileInfo fileInfo(dlFilePath);
    QTableWidgetItem *filePathItem = new QTableWidgetItem(fileInfo.fileName());
    filePathItem->setToolTip(dlFilePath);
    filePathItem->setFlags(opItem->flags() & ~Qt::ItemIsEditable);
    ui->transferingList->setItem(rowIndex, Columns::File, filePathItem);

    qint64 totalProgress = fileUploadTask->totalProgress();
    qint64 currentProgress = fileUploadTask->currentProgress();
    QString transferredStr;
    if (totalProgress > 0) {
        transferredStr = transferredString(currentProgress, totalProgress);
    } else {
        transferredStr = transferredString(0, fileUploadTask->uploadFileSize());
    }
    QTableWidgetItem *transferredItem = new QTableWidgetItem(transferredStr);
    transferredItem->setFlags(opItem->flags() & ~Qt::ItemIsEditable);
    ui->transferingList->setItem(rowIndex, Columns::Transferred, transferredItem);
    QObject::connect(fileUploadTask,
                     &Task::progress,
                     this,
                     [this, transferredItem, fileUploadTask]() {
                         qint64 totalProgress = fileUploadTask->totalProgress();
                         qint64 currentProgress = fileUploadTask->currentProgress();
                         transferredItem->setText(transferredString(currentProgress, totalProgress));
                     });

    QTableWidgetItem *statusItem = new QTableWidgetItem(
        taskStatusString(fileUploadTask->taskStatus()));
    statusItem->setFlags(opItem->flags() & ~Qt::ItemIsEditable);
    ui->transferingList->setItem(rowIndex, Columns::Progress, statusItem);
    QObject::connect(fileUploadTask,
                     &Task::statusChanged,
                     this,
                     [this, statusItem, fileUploadTask]() {
                         statusItem->setText(taskStatusString(fileUploadTask->taskStatus()));
                     });

    QWidget *w = new QWidget(ui->transferingList);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    QPushButton *removeButton = new QPushButton(w);
    removeButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    removeButton->setToolTip(tr("Delete"));
    layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout->addWidget(removeButton);
    layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    w->setFixedWidth(40);
    ui->transferingList->setCellWidget(rowIndex, Columns::Cancel, w);
    QObject::connect(removeButton, &QPushButton::clicked, this, [this, fileUploadTask]() {
        if (!fileUploadTask->isRunning())
            m_sendFilesTaskSchedule->removeTask(fileUploadTask->sharedFromThis());
        else
            m_sendFilesTaskSchedule->abortTask(fileUploadTask->sharedFromThis());
    });
}

int FileTransferPage::findListRowIndexByTask(Task *task)
{
    int rowCount = ui->transferingList->rowCount();
    for (int i = 0; i < rowCount; i++) {
        auto item = ui->transferingList->item(i, Columns::Operation);
        Task *t = qvariant_cast<Task *>(item->data(Qt::UserRole));
        if (t == task) {
            return i;
        }
    }
    return -1;
}

QString FileTransferPage::transferredString(qint64 current, qint64 total)
{
    return formatedSizeString(current) + QLatin1Char('/') + formatedSizeString(total);
}

QString FileTransferPage::formatedSizeString(qint64 val)
{
    QString str;
    if (val >> 30 > 0) {
        str = QString::number(static_cast<double>(val) / 1_GB, 'g');
        str = trimDecimals(str, 2) + QLatin1StringView("GB");
    } else if (val >> 20 > 0) {
        str = QString::number(static_cast<double>(val) / 1_MB, 'g');
        str = trimDecimals(str, 2) + QLatin1StringView("MB");
    } else if (val >> 10 > 0) {
        str = QString::number(static_cast<double>(val) / 1_KB, 'g');
        str = trimDecimals(str, 2) + QLatin1StringView("KB");
    } else if (val > 0) {
        str = QLatin1StringView("1KB");
    } else {
        str = QLatin1StringView("0KB");
    }

    return str;
}

void FileTransferPage::loadTransferHistoryList()
{
    if (m_transferHistoryManager) {
        qint64 finishTime = -1;
        if (m_transferHistoryListModel->rowCount() > 0) {
            auto index = m_transferHistoryListModel->index(m_transferHistoryListModel->rowCount()
                                                           - 1);
            finishTime = qvariant_cast<qint64>(
                m_transferHistoryListModel->data(index,
                                                 TransferHistoryListItemDataRoles::FinishTime));
        }

        QList<TransferHistoryRecord> records = m_transferHistoryManager->getHistories(finishTime);

        if (m_transferHistoryListModel->appendData(records) == 0) {
            auto scrollBar = ui->historyList->verticalScrollBar();
            QObject::disconnect(scrollBar,
                                &QScrollBar::valueChanged,
                                this,
                                &FileTransferPage::onHistoryListVerticalScrollbarValueChanged);
        }
    }
}

void FileTransferPage::createHistoryListMenu(const QPoint &pos)
{
    auto index = ui->historyList->indexAt(pos);
    if (index.isValid()) {
        QMenu *menu = new QMenu();
        QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(
            ui->historyList->model());
        if (proxyModel != nullptr) {
            index = proxyModel->mapToSource(index);
        }
        QString filePath = m_transferHistoryListModel
                               ->data(index, TransferHistoryListItemDataRoles::File)
                               .toString();
        qint64 id = qvariant_cast<qint64>(
            m_transferHistoryListModel->data(index, TransferHistoryListItemDataRoles::Id));

        int result = qvariant_cast<int>(
            m_transferHistoryListModel->data(index, TransferHistoryListItemDataRoles::Result));
        if (result == TransferHistoryRecord::Result::SuccessFul) {
            auto fileOpenAction = menu->addAction(tr("Open File"));
            QObject::connect(fileOpenAction, &QAction::triggered, this, [filePath]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
            });

            auto fileDirOpenAction = menu->addAction(tr("Open Folder"));
            QObject::connect(fileDirOpenAction, &QAction::triggered, this, [filePath]() {
                QFileInfo fileInfo(filePath);
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
            });

            menu->addSeparator();
        }

        auto removeThisRecord = menu->addAction(tr("Remove this history item"));
        QObject::connect(removeThisRecord, &QAction::triggered, this, [this, id]() {
            if (m_transferHistoryManager)
                m_transferHistoryManager->removeHistory(id);
        });

        auto clearAllRecords = menu->addAction(tr("Clear all histories"));
        QObject::connect(clearAllRecords, &QAction::triggered, this, [this]() {
            if (m_transferHistoryManager)
                m_transferHistoryManager->clearHistories();
        });

        menu->exec(ui->historyList->mapToGlobal(pos));
        menu->deleteLater();
    }
}

void FileTransferPage::clearTransferHistoryList()
{
    m_transferHistoryListModel->onClear();
}

void FileTransferPage::onHistoryListVerticalScrollbarValueChanged(int value)
{
    if (QScrollBar *scrollBar = qobject_cast<QScrollBar *>(QObject::sender());
        scrollBar != nullptr) {
        float percent = (value - scrollBar->minimum())
                        / static_cast<float>(scrollBar->maximum() - scrollBar->minimum());

        if (percent >= 0.75f) {
            QMetaObject::invokeMethod(this,
                                      &FileTransferPage::loadTransferHistoryList,
                                      Qt::QueuedConnection);
        }
    }
}

void FileTransferPage::onHistoryItemActivated(const QModelIndex &index)
{
    if (index.isValid()) {
        auto sourceIndex = index;
        QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(
            ui->historyList->model());
        if (proxyModel != nullptr) {
            sourceIndex = proxyModel->mapToSource(index);
        }

        QString filePath = m_transferHistoryListModel
                               ->data(sourceIndex, TransferHistoryListItemDataRoles::File)
                               .toString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
}
