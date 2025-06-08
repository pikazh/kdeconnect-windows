#pragma once

#include <QSharedPointer>
#include <QWidget>

#include "plugin/sharepluginwrapper.h"
#include "transferhistorylistmodel.h"
#include "ui/pages/BasePage.h"

#include "core/device.h"

#include "core/task/peerfiledownloadtask.h"
#include "core/task/peerfileuploadtask.h"

#include "core/plugins/share/itransferhistorymanager.h"

namespace Ui {
class FileTransferPage;
}

class FileTransferPage : public QWidget, public BasePage
{
    Q_OBJECT
public:
    explicit FileTransferPage(Device::Ptr device, QWidget *parent = nullptr);
    virtual ~FileTransferPage() override;

    virtual QString displayName() const override { return tr("File Share"); }
    virtual QIcon icon() const override;
    virtual QString id() const override { return QLatin1StringView("File Share"); }
    virtual bool apply() override;
    virtual QString helpPage() const override { return ""; }
    virtual bool shouldDisplay() const override;
    virtual void retranslate() override;

protected:
    enum Columns {
        Operation = 0,
        File,
        Transferred,
        Progress,
        Cancel,
        Count,
    };

    void initTransferingList();

    void addTransferingListItem(PeerFileDownloadTask *task);
    void addTransferingListItem(PeerFileUploadTask *task);

    int findListRowIndexByTask(Task *task);

    void deleteAllRowsFromTransferingList();

    QString transferredString(qint64 current, qint64 total);
    QString formatedSizeString(qint64 val);

    void clearTransferHistoryList();

protected Q_SLOTS:
    void onHistoryListVerticalScrollbarValueChanged(int value);
    void onHistoryItemActivated(const QModelIndex &index);

    void loadTransferHistoryList();
    void createHistoryListMenu(const QPoint &pos);

private:
    Ui::FileTransferPage *ui;
    SharePluginWrapper *m_sharePluginWrapper = nullptr;
    QSharedPointer<TaskScheduler> m_recvFilesTaskSchedule;
    QSharedPointer<TaskScheduler> m_sendFilesTaskSchedule;
    QSharedPointer<ITransferHistoryManager> m_transferHistoryManager;

    TransferHistoryListModel *m_transferHistoryListModel;
};
