#include "smsconversationcontentwidget.h"
#include "app_debug.h"
#include "smscontentitem.h"
#include "smshelper.h"
#include "uicommon.h"
#include "ui_smsconversationcontentwidget.h"

#include <QFileDialog>
#include <QIcon>
#include <QScrollBar>
#include <QTreeWidgetItem>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

#define AVATAR_WIDTH (32)

using namespace std::chrono_literals;

SmsConversationContentWidget::SmsConversationContentWidget(SmsManager *smsManager,
                                                           qint64 conversationId,
                                                           const QList<QString> &addresses,
                                                           qint64 simcardSubId,
                                                           ContactProvider *contactProvider,
                                                           QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::SmsConversationContentWidget)
    , m_smsManager(smsManager)
    , m_conversationId(conversationId)
    , m_addresses(addresses)
    , m_simcardSubId(simcardSubId)
    , m_contactProvider(contactProvider)
    , m_lastCheckMsgTime(std::chrono::system_clock::now())
    , m_checkMsgDuration(1000ms)
{
    ui->setupUi(this);
    ui->sendButton->setIcon(RETRIEVE_THEME_ICON("document-send"));
    ui->attachmentButton->setIcon(RETRIEVE_THEME_ICON("mail-attachment"));
    ui->attachmentList->init();
    ui->attachmentList->hide();

    QObject::connect(ui->attachmentList,
                     &SmsAttachmentListWidget::attachmentAdded,
                     this,
                     &SmsConversationContentWidget::onAttachmentSendListAddedItem);
    QObject::connect(ui->attachmentList,
                     &SmsAttachmentListWidget::attachmentRemoved,
                     this,
                     &SmsConversationContentWidget::onAttachmentSendListRemovedItem);

    ui->splitter->setCollapsible(0, false);

    auto font = ui->title->font();
    font.setPointSize(font.pointSize() + 2);
    ui->title->setFont(font);

    m_defaultAvator = RETRIEVE_THEME_ICON("im-user").pixmap(AVATAR_WIDTH, AVATAR_WIDTH);

    ui->listItemWidget->setColumnCount(Columns::Count);
    ui->listItemWidget->hideColumn(Columns::Time);
    ui->listItemWidget->sortItems(Columns::Time, Qt::AscendingOrder);
    ui->listItemWidget->installEventFilter(this);

    auto scrollBar = ui->listItemWidget->verticalScrollBar();
    scrollBar->installEventFilter(this);
    QObject::connect(scrollBar,
                     &QScrollBar::valueChanged,
                     this,
                     &SmsConversationContentWidget::onContentListVerticalScrollbarValueChanged);

    QObject::connect(m_smsManager,
                     &SmsManager::conversationNewMessage,
                     this,
                     &SmsConversationContentWidget::onConversationNewMessage);

    QObject::connect(m_contactProvider,
                     &ContactProvider::contactUpdated,
                     this,
                     &SmsConversationContentWidget::onContactUpdated);

    loadMessages();
}

SmsConversationContentWidget::~SmsConversationContentWidget()
{
    delete ui;
}

void SmsConversationContentWidget::setTitle(const QString &title)
{
    ui->title->setText(title);
}

void SmsConversationContentWidget::bindToConversation(qint64 newConversationId)
{
    Q_ASSERT(newConversationId > -1);
    if (m_conversationId != newConversationId) {
        m_conversationId = newConversationId;

        ui->listItemWidget->clear();

        loadMessages();
    }
}

void SmsConversationContentWidget::addMessage(const ConversationMessage &message,
                                              const int insertedIndex)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    SmsContentItem *contentItemWidget = new SmsContentItem(ui->listItemWidget);
    item->setSizeHint(Columns::Content,
                      QSize(ui->listItemWidget->width(), contentItemWidget->size().height()));

    // for sorting
    item->setText(Columns::Time, QString::number(message.date()));

    contentItemWidget->setTime(message.date());

    contentItemWidget->setText(message.body());
    if (message.isOutgoing()) {
        contentItemWidget->setName(tr("You"));
        contentItemWidget->setAvatar(m_defaultAvator);
    } else {
        QString senderName = message.addresses().at(0).address();
        contentItemWidget->setSenderNumber(senderName);

        QString canonicalizedNumber = SMSHelper::canonicalizePhoneNumber(senderName);
        auto contact = m_contactProvider->lookupContactByPhoneNumber(canonicalizedNumber);
        if (!contact.isEmpty()) {
            contentItemWidget->setName(contact.realName());
            if (m_cachedAvatars.contains(canonicalizedNumber)) {
                contentItemWidget->setAvatar(m_cachedAvatars[canonicalizedNumber]);
            } else if (!contact.photo().isEmpty()) {
                auto avatar = QPixmap::fromImage(contact.photo().data())
                                  .scaled(AVATAR_WIDTH,
                                          AVATAR_WIDTH,
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
                m_cachedAvatars[canonicalizedNumber] = avatar;
                contentItemWidget->setAvatar(avatar);
            } else {
                contentItemWidget->setAvatar(m_defaultAvator);
            }

        } else {
            contentItemWidget->setName(senderName);
            contentItemWidget->setAvatar(m_defaultAvator);
        }
    }

    if (!message.attachments().isEmpty()) {
        contentItemWidget->setAttachments(message.uID(), message.attachments(), m_smsManager);
    }

    ui->listItemWidget->addTopLevelItem(item);
    ui->listItemWidget->setItemWidget(item, Columns::Content, contentItemWidget);

    if (insertedIndex == 0)
        ui->listItemWidget->scrollToBottom();
}

void SmsConversationContentWidget::on_attachmentButton_clicked()
{
    QStringList selected = QFileDialog::getOpenFileNames(this);
    ui->attachmentList->addAttachmentList(selected);
}

void SmsConversationContentWidget::on_sendButton_clicked()
{
    QStringList attachmentList = ui->attachmentList->attachmentList();
    ui->attachmentList->clearAttachmentList();
    ui->attachmentList->hide();

    QString msg = ui->smsInputEdit->toPlainText().trimmed();
    ui->smsInputEdit->clear();
    m_smsManager->sendSms(m_addresses, msg, attachmentList, m_simcardSubId);
}

void SmsConversationContentWidget::onAttachmentSendListRemovedItem(const QString &path)
{
    if (ui->attachmentList->isAttachmentListEmpty()) {
        if (ui->attachmentList->isVisible()) {
            ui->attachmentList->hide();
        }
    }
}

void SmsConversationContentWidget::onAttachmentSendListAddedItem(const QString &path)
{
    if (!ui->attachmentList->isVisible()) {
        ui->attachmentList->show();
    }
}

void SmsConversationContentWidget::onConversationNewMessage(const qint64 conversationId,
                                                            const ConversationMessage &msg,
                                                            const int insertedIndex)
{
    if (conversationId == m_conversationId) {
        addMessage(msg, insertedIndex);
        m_checkMsgDuration = 1000ms;
    }
}

void SmsConversationContentWidget::onContactUpdated()
{
    m_cachedAvatars.clear();

    for (int i = 0; i < ui->listItemWidget->topLevelItemCount(); ++i) {
        auto item = ui->listItemWidget->topLevelItem(i);
        SmsContentItem *itemWidget = qobject_cast<SmsContentItem *>(
            ui->listItemWidget->itemWidget(item, Columns::Content));
        Q_ASSERT(itemWidget != nullptr);
        if (itemWidget != nullptr) {
            QString senderNumber = itemWidget->senderNumber();
            if (!senderNumber.isEmpty()) {
                QString canonicalizedNumber = SMSHelper::canonicalizePhoneNumber(senderNumber);
                auto contact = m_contactProvider->lookupContactByPhoneNumber(senderNumber);
                if (!contact.isEmpty()) {
                    itemWidget->setName(contact.realName());
                    if (m_cachedAvatars.contains(canonicalizedNumber)) {
                        itemWidget->setAvatar(m_cachedAvatars[canonicalizedNumber]);
                    } else if (!contact.photo().isEmpty()) {
                        auto avatar = QPixmap::fromImage(contact.photo().data())
                                          .scaled(AVATAR_WIDTH,
                                                  AVATAR_WIDTH,
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation);
                        m_cachedAvatars[canonicalizedNumber] = avatar;
                        itemWidget->setAvatar(avatar);
                    } else {
                        itemWidget->setAvatar(m_defaultAvator);
                    }

                } else {
                    itemWidget->setName(senderNumber);
                    itemWidget->setAvatar(m_defaultAvator);
                }
            }
        }
    }
}

void SmsConversationContentWidget::adjustItemsSize()
{
    for (int i = 0; i < ui->listItemWidget->topLevelItemCount(); ++i) {
        auto item = ui->listItemWidget->topLevelItem(i);
        auto itemWidget = ui->listItemWidget->itemWidget(item, Columns::Content);
        auto width = ui->listItemWidget->width() - ui->listItemWidget->verticalScrollBar()->width();
        item->setSizeHint(Columns::Content, QSize(width, itemWidget->size().height()));
    }
}

void SmsConversationContentWidget::loadMessages()
{
    if (m_conversationId > -1) {
        const auto [hasConversation, msgList] = m_smsManager->conversationMessages(m_conversationId);
        Q_ASSERT(hasConversation);
        if (hasConversation) {
            for (auto i = 0; i < msgList.count(); ++i) {
                addMessage(msgList[i], i);
            }

            if (msgList.count() < 20) {
                m_smsManager->requireMoreMessages(m_conversationId, msgList.last().date(), 20);
            }
        }
    }
}

bool SmsConversationContentWidget::eventFilter(QObject *obj, QEvent *event)
{
    auto evtType = event->type();
    if (obj == ui->listItemWidget) {
        if (evtType == QEvent::Resize || evtType == QEvent::Show) {
            QMetaObject::invokeMethod(this,
                                      &SmsConversationContentWidget::adjustItemsSize,
                                      Qt::QueuedConnection);
        }
    } else if (obj == ui->listItemWidget->verticalScrollBar()) {
        if (evtType == QEvent::Hide || evtType == QEvent::Show) {
            QMetaObject::invokeMethod(this,
                                      &SmsConversationContentWidget::adjustItemsSize,
                                      Qt::QueuedConnection);
        }
    }

    return QWidget::eventFilter(obj, event);
}

void SmsConversationContentWidget::onContentListVerticalScrollbarValueChanged(int value)
{
    qDebug(KDECONNECT_APP) << value;
    if (value == 0) {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> dur = now - m_lastCheckMsgTime;
        if (dur > m_checkMsgDuration) {
            m_lastCheckMsgTime = now;
            m_checkMsgDuration = m_checkMsgDuration * 2;
            if (m_checkMsgDuration > 10000ms)
                m_checkMsgDuration = 10000ms;

            const auto [hasConversation, msgList] = m_smsManager->conversationMessages(
                m_conversationId);
            Q_ASSERT(hasConversation);
            m_smsManager->requireMoreMessages(m_conversationId, msgList.last().date(), 20);
        }
    }
}
