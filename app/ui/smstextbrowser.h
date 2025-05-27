#pragma once

#include <QString>
#include <QTextBrowser>

#include "core/sms/conversationmessage.h"

#include "smsmanager.h"

class SmsTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    SmsTextBrowser(QWidget *parent = nullptr);

    virtual QVariant loadResource(int type, const QUrl &name) override;

    void setPlainContent(const QString &plainContent);

    void setAttachments(const qint32 msgId,
                        const QList<Attachment> &attachments,
                        SmsManager *smsManager);

protected Q_SLOTS:
    void onAttachmentDownloadFinished(qint32 msgId,
                                      qint64 partId,
                                      bool success,
                                      const QString &fileName);

    void onAnchorClicked(const QUrl &link);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

    enum AttachmentDownloadState {
        Downloading = 0,
        DownloadSuccess,
        DownloadFailed,
    };

    struct AttachmentExtraInfo
    {
        AttachmentDownloadState dlstate;
        QString thumbnailName;
        QString downloadedFileName;
    };

    void updateAttachmentInfo(qint32 msgId, qint64 partId, bool success, const QString &fileName);
    void updateText();

private:
    qint32 m_associatedMsgId = -1;
    QList<Attachment> m_attachments;
    QList<AttachmentExtraInfo> m_attachmentInfos;

    SmsManager *m_smsManager = nullptr;

    QString m_AttachmentHtmlContent;
    QString m_plainContent;
};
