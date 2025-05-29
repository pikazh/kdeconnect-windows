#include "smstextbrowser.h"
#include "app_debug.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QFileInfo>
#include <QLatin1StringView>
#include <QMenu>
#include <QMimeDatabase>
#include <QMimeType>
#include <QString>

SmsTextBrowser::SmsTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    QObject::connect(this, &SmsTextBrowser::anchorClicked, this, &SmsTextBrowser::onAnchorClicked);
}

QVariant SmsTextBrowser::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource) {
        QString resName = name.toString();
        for (int i = 0; i < m_attachmentInfos.size(); ++i) {
            if (m_attachmentInfos[i].thumbnailName == resName) {
                if (!m_attachments[i].base64EncodedFile().isEmpty()) {
                    const QByteArray byteArray = m_attachments[i].base64EncodedFile().toUtf8();
                    return QByteArray::fromBase64(byteArray);
                } else {
                    QMimeDatabase db;
                    auto mimeType = db.mimeTypeForName(m_attachments[i].mimeType());
                    qDebug(KDECONNECT_APP) << "icon name:" << mimeType.iconName();
                    auto icon = QIcon::fromTheme(mimeType.iconName());
                    if (icon.isNull()) {
                        qDebug(KDECONNECT_APP)
                            << "generic icon name:" << mimeType.genericIconName();
                        icon = QIcon::fromTheme(mimeType.genericIconName());
                    }
                    return icon.pixmap(48, 48);
                }

                break;
            }
        }
    }

    return QTextBrowser::loadResource(type, name);
}

void SmsTextBrowser::setPlainContent(const QString &plainContent)
{
    m_plainContent = plainContent;
    this->setText(m_AttachmentHtmlContent + m_plainContent);
}

void SmsTextBrowser::setAttachments(const qint32 msgId,
                                    const QList<Attachment> &attachments,
                                    SmsManager *smsManager)
{
    Q_ASSERT(msgId >= 0 && smsManager != nullptr && !attachments.isEmpty());
    if (m_smsManager != nullptr) {
        QObject::disconnect(m_smsManager, nullptr, this, nullptr);
    }
    m_associatedMsgId = msgId;
    m_smsManager = smsManager;
    m_attachments = attachments;
    m_attachmentInfos.resize(m_attachments.size());

    QObject::connect(smsManager,
                     &SmsManager::attachmentDownloadFinished,
                     this,
                     &SmsTextBrowser::onAttachmentDownloadFinished,
                     Qt::UniqueConnection);

    for (int i = 0; i < m_attachments.size(); ++i) {
        Q_ASSERT(attachments[i].isValid());

        m_attachmentInfos[i].thumbnailName
            = QStringLiteral("%1_%2.thumbnail").arg(msgId).arg(attachments[i].partID());

        m_attachmentInfos[i].dlstate = AttachmentDownloadState::Downloading;
        m_attachmentInfos[i].downloadedFileName.clear();

        m_smsManager->downloadAttachment(m_associatedMsgId, m_attachments[i]);
    }

    updateText();
}

void SmsTextBrowser::onAttachmentDownloadFinished(qint32 msgId,
                                                  qint64 partId,
                                                  bool success,
                                                  const QString &fileName)
{
    if (msgId == this->m_associatedMsgId) {
        updateAttachmentInfo(msgId, partId, success, fileName);
    }
}

void SmsTextBrowser::onAnchorClicked(const QUrl &link)
{
    if (link.isRelative()) {
        QString fileName = link.toString();
        QString fullPath = m_smsManager->attachmentDownloadDir() + QDir::separator() + fileName;
        QFileInfo fileInfo(fullPath);
        if (fileInfo.isFile()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(fullPath));
        }
    }
}

void SmsTextBrowser::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu(event->pos());
    if (!m_attachments.isEmpty()) {
        menu->addSeparator();
        menu->addAction(tr("Open Attachment Folder"), this, [this]() {
            QDesktopServices::openUrl(m_smsManager->attachmentDownloadDir());
        });
    }
    menu->exec(event->globalPos());
    delete menu;
}

void SmsTextBrowser::updateAttachmentInfo(qint32 msgId,
                                          qint64 partId,
                                          bool success,
                                          const QString &fileName)
{
    for (int i = 0; i < m_attachments.size(); ++i) {
        if (m_attachments[i].partID() == partId) {
            m_attachmentInfos[i].dlstate = success ? AttachmentDownloadState::DownloadSuccess
                                                   : AttachmentDownloadState::DownloadFailed;
            if (success) {
                m_attachmentInfos[i].downloadedFileName = fileName;
            }

            break;
        }
    }

    updateText();
}

void SmsTextBrowser::updateText()
{
    m_AttachmentHtmlContent = QLatin1StringView("<style>"
                                                "td, th {"
                                                "text-align: center;"
                                                "}"
                                                "</style>"
                                                "<table>"
                                                "<tr>");

    m_AttachmentHtmlContent.append(QStringLiteral("<td>"))
        .append(tr("Attachment(s):"))
        .append(QStringLiteral("</td>"));
    for (int i = 0; i < m_attachments.size(); ++i) {
        if (m_attachmentInfos[i].dlstate == AttachmentDownloadState::DownloadSuccess) {
            m_AttachmentHtmlContent.append(
                QStringLiteral("<td><a href=\"%1\"><img src=\"%2\"></a></td>")
                    .arg(m_attachmentInfos[i].downloadedFileName)
                    .arg(m_attachmentInfos[i].thumbnailName));
        } else {
            m_AttachmentHtmlContent.append(
                QStringLiteral("<td><img src=\"%1\"></td>").arg(m_attachmentInfos[i].thumbnailName));
        }
    }

    m_AttachmentHtmlContent.append(QLatin1StringView("</tr>"
                                                     "<tr>"
                                                     "<td>"
                                                     "</td>"));

    QString tips;
    for (int i = 0; i < m_attachments.size(); ++i) {
        switch (m_attachmentInfos[i].dlstate) {
        case AttachmentDownloadState::Downloading:
            tips = tr("Downloading...");
            break;
        case AttachmentDownloadState::DownloadFailed:
            tips = tr("Download Failed");
            break;
        case AttachmentDownloadState::DownloadSuccess:
            tips = m_attachmentInfos[i].downloadedFileName;
            break;
        }

        m_AttachmentHtmlContent.append(QLatin1StringView("<td>") + tips
                                       + QLatin1StringView("</td>"));
    }

    m_AttachmentHtmlContent.append(QLatin1StringView("</tr>"
                                                     "</table>"
                                                     "<br />"));

    this->setText(m_AttachmentHtmlContent + m_plainContent);
}
