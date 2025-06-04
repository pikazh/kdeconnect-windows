#include "filetransferpage.h"
#include "ui_filetransferpage.h"

#include "volumedeviceitem.h"

#include "core/task/peerfiledownloadtask.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPushButton>
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

    auto initFunctor = [this]() {
        m_recvFilesTaskSchedule = m_sharePluginWrapper->recvFilesTaskSchedule();
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
                             PeerFileDownloadTask *t = qobject_cast<PeerFileDownloadTask *>(
                                 task.get());
                             int rowIndex = findListRowIndexByTask(t);
                             Q_ASSERT(rowIndex > -1);
                             if (rowIndex > -1)
                                 ui->transferingList->removeRow(rowIndex);
                         });

        initTransferingList();
    };

    QObject::connect(m_sharePluginWrapper,
                     &SharePluginWrapper::pluginLoadedChange,
                     this,
                     [this, initFunctor](bool loaded) {
                         if (loaded) {
                             initFunctor();
                         } else {
                             deleteAllRowsFromTransferingList();
                             m_recvFilesTaskSchedule.reset();
                             m_sendFilesTaskSchedule.reset();
                         }
                     });

    m_sharePluginWrapper->init();

    initFunctor();
}

FileTransferPage::~FileTransferPage()
{
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
    QObject::connect(fileDlTask,
                     &PeerFileDownloadTask::progress,
                     this,
                     [this, transferredItem, fileDlTask]() {
                         qint64 totalProgress = fileDlTask->totalProgress();
                         qint64 currentProgress = fileDlTask->currentProgress();
                         transferredItem->setText(transferredString(currentProgress, totalProgress));
                     });

    QTableWidgetItem *statusItem = new QTableWidgetItem(taskStatusString(fileDlTask->taskStatus()));
    statusItem->setFlags(opItem->flags() & ~Qt::ItemIsEditable);
    ui->transferingList->setItem(rowIndex, Columns::Progress, statusItem);
    QObject::connect(fileDlTask,
                     &PeerFileDownloadTask::statusChanged,
                     this,
                     [this, statusItem, fileDlTask]() {
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

int FileTransferPage::findListRowIndexByTask(PeerFileDownloadTask *task)
{
    int rowCount = ui->transferingList->rowCount();
    for (int i = 0; i < rowCount; i++) {
        auto item = ui->transferingList->item(i, Columns::Operation);
        PeerFileDownloadTask *t = qvariant_cast<PeerFileDownloadTask *>(item->data(Qt::UserRole));
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
