#include "smsconversationswidget.h"
#include "smsconversationcontentwidget.h"
#include "smsconversationlistitemdelegate.h"
#include "smshelper.h"
#include "smsitemdata.h"
#include "smslistitemsortfilterproxymodel.h"
#include "ui_smswidget.h"

#include "core/imageutil.h"

#include <QLatin1StringView>
#include <QStandardItem>
#include <QStandardItemModel>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

const int g_listItemHeight = 40;
const int g_listItemIconWidth = g_listItemHeight - 6;

SmsConversationsWidget::SmsConversationsWidget(Device::Ptr dev, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SMSWidget)
    , m_smsPluginWrapper(new SmsPluginWrapper(dev, this))
    , m_contactProvider(new ContactProvider(dev, this))
{
    ui->setupUi(this);
    //ui->addConversationButton->setIcon(RETRIEVE_THEME_ICON("list-add"));

    ui->conversationContentWidgets->addWidget(
        new SmsConversationContentWidget(ui->conversationContentWidgets));
    ui->conversationContentWidgets->setCurrentIndex(0);

    //ui->splitter->setStretchFactor(0, 3);
    //ui->splitter->setStretchFactor(1, 5);

    SmsListItemSortFilterProxyModel *proxyModel = new SmsListItemSortFilterProxyModel(
        ui->conversationListView);
    proxyModel->setSourceModel(new QStandardItemModel(ui->conversationListView));
    ui->conversationListView->setModel(proxyModel);
    ui->conversationListView->setItemDelegate(
        new SMSConversationListItemDelegate(ui->conversationListView));

    QObject::connect(ui->conversationListView->selectionModel(),
                     &QItemSelectionModel::currentChanged,
                     this,
                     &SmsConversationsWidget::onConversationListSelectionChanged);

    m_smsPluginWrapper->init();

    QObject::connect(m_smsPluginWrapper,
                     &SmsPluginWrapper::messagesReceived,
                     this,
                     &SmsConversationsWidget::onSmsMessagesReceived);

    QObject::connect(m_contactProvider,
                     &ContactProvider::contactUpdated,
                     this,
                     &SmsConversationsWidget::onContactUpdated);

    m_smsPluginWrapper->requestAllConversations();
}

SmsConversationsWidget::~SmsConversationsWidget()
{
    delete ui;
}

void SmsConversationsWidget::refreshConversation()
{
    m_smsPluginWrapper->requestAllConversations();
}

void SmsConversationsWidget::onSmsMessagesReceived(const QList<ConversationMessage> &msgList)
{
    for (int i = 0; i < msgList.length(); ++i) {
        bool newlyCreated = false;
        int insertedIndex = -1;
        qint64 conversationId = msgList[i].threadID();
        auto it = m_conversations.find(conversationId);
        if (it == m_conversations.end()) {
            m_conversations.insert(conversationId, QList<ConversationMessage>{msgList[i]});
            newlyCreated = true;
        } else {
            QList<ConversationMessage> &conversation = it.value();
            int j = 0;
            for (; j < conversation.size(); ++j) {
                if (msgList[i].date() >= conversation[j].date()) {
                    if (msgList[i].uID() != conversation[j].uID()) {
                        conversation.insert(j, msgList[i]);
                        insertedIndex = j;
                    }
                    break;
                }
            }
            if (j == conversation.size()) {
                conversation.push_back(msgList[i]);
                insertedIndex = j;
            }
        }

        if (newlyCreated) {
            createConversationListItem(conversationId);
        } else if (insertedIndex > -1) {
            onConversationUpdated(conversationId, insertedIndex);
        }
    }
}

void SmsConversationsWidget::onContactUpdated()
{
    uiUpdateConversationsContactInfo();
}

void SmsConversationsWidget::onConversationListSelectionChanged(const QModelIndex &current,
                                                                const QModelIndex &previous)
{
    SmsListItemData::Ptr itemData = qvariant_cast<SmsListItemData::Ptr>(
        ui->conversationListView->model()->data(current, SmsListItemDataRoles::Data));
    if (itemData) {
        if (itemData->conversationContentWidgetIndex >= 0) {
            ui->conversationContentWidgets->setCurrentIndex(
                itemData->conversationContentWidgetIndex);

        } else {
            SmsConversationContentWidget *contentWidget = new SmsConversationContentWidget(
                ui->conversationContentWidgets);
            itemData->conversationContentWidgetIndex = ui->conversationContentWidgets->addWidget(
                contentWidget);
            ui->conversationContentWidgets->setCurrentWidget(contentWidget);

            contentWidget->setTitle(
                ui->conversationListView->model()->data(current, Qt::DisplayRole).toString());

            if (itemData->conversationId >= 0) {
                auto &msgList = m_conversations[itemData->conversationId];
                for (auto i = 0; i < msgList.size(); ++i) {
                    contentWidget->addMessage(i, msgList.at(i));
                }

                if (msgList.size() < 10) {
                    m_smsPluginWrapper->requestConversation(itemData->conversationId,
                                                            msgList.last().date(),
                                                            10);
                }

                contentWidget->setProperty("conversationId", itemData->conversationId);

                QObject::connect(contentWidget,
                                 &SmsConversationContentWidget::requestMoreMessages,
                                 this,
                                 [this]() {
                                     QObject *sender = QObject::sender();
                                     auto conversationId = qvariant_cast<qint64>(
                                         sender->property("conversationId"));
                                     if (conversationId >= 0) {
                                         auto &msgList = m_conversations[conversationId];
                                         m_smsPluginWrapper
                                             ->requestConversation(conversationId,
                                                                   msgList.last().date(),
                                                                   10);
                                     }
                                 });
            }
        }
    }
}

void SmsConversationsWidget::createConversationListItem(qint64 conversationId)
{
    QAbstractProxyModel *proxyModel = qobject_cast<QAbstractProxyModel *>(
        ui->conversationListView->model());
    QStandardItemModel *itemModel = qobject_cast<QStandardItemModel *>(proxyModel->sourceModel());
    int rowCount = itemModel->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        SmsListItemData::Ptr itemData = qvariant_cast<SmsListItemData::Ptr>(
            itemModel->item(i)->data(SmsListItemDataRoles::Data));

        Q_ASSERT(itemData);

        if (itemData->conversationId == conversationId) {
            return;
        }
    }

    QStandardItem *newItem = new QStandardItem();
    newItem->setEditable(false);
    ConversationMessage &conversationMsg = m_conversations[conversationId].first();

    SmsListItemData::Ptr itemData(new SmsListItemData);
    itemData->conversationId = conversationId;
    itemData->latestMsgTime = conversationMsg.date();
    QList<ConversationAddress> conversationLists = conversationMsg.addresses();
    for (int i = 0; i < conversationLists.count(); ++i) {
        itemData->canonicalizedPhoneNumbers.append(
            SMSHelper::canonicalizePhoneNumber(conversationLists[i].address()));
    }

    newItem->setData(QVariant::fromValue(itemData), SmsListItemDataRoles::Data);
    newItem->setData(QSize(0, g_listItemHeight), Qt::SizeHintRole);

    QList<QPixmap> avatars;
    auto defaultAvatar = QIcon::fromTheme(QStringLiteral("im-user")).pixmap(1024, 1024);
    QStringList contactNameAndNumberList;
    for (int i = 0; i < itemData->canonicalizedPhoneNumbers.count(); ++i) {
        auto contact = m_contactProvider->lookupContactByPhoneNumberOrName(
            itemData->canonicalizedPhoneNumbers[i]);
        if (!contact.isEmpty()) {
            contactNameAndNumberList.push_back(contact.realName() + QLatin1StringView("(")
                                               + itemData->canonicalizedPhoneNumbers[i]
                                               + QLatin1StringView(")"));

            if (contact.photo().isEmpty()) {
                avatars.append(defaultAvatar);
            } else {
                avatars.append(QPixmap::fromImage(contact.photo().data()));
            }

        } else {
            contactNameAndNumberList.push_back(itemData->canonicalizedPhoneNumbers[i]);

            avatars.append(defaultAvatar);
        }
    }

    newItem->setData(ImageUtil::combineImage(avatars).scaled(g_listItemIconWidth,
                                                             g_listItemIconWidth,
                                                             Qt::KeepAspectRatio,
                                                             Qt::SmoothTransformation),
                     Qt::DecorationRole);

    newItem->setData(contactNameAndNumberList.join(QLatin1StringView(", ")), Qt::DisplayRole);

    QString senderName;
    // If the message is incoming, the sender is the first Address]
    if (conversationMsg.isOutgoing()) {
        senderName = tr("You");
    } else {
        auto phoneNumber = SMSHelper::canonicalizePhoneNumber(conversationLists[0].address());
        auto contact = m_contactProvider->lookupContactByPhoneNumberOrName(phoneNumber);
        if (!contact.isEmpty()) {
            senderName = contact.realName();
        } else {
            senderName = phoneNumber;
        }
    }

    QString msgPreview = QLatin1StringView("%1: %2").arg(senderName).arg(conversationMsg.body());

    if (!conversationMsg.attachments().isEmpty()) {
        if (!conversationMsg.body().isEmpty()) {
            msgPreview = msgPreview + QStringLiteral(" ") + tr("[Attachment]");
        } else {
            msgPreview = msgPreview + tr("[Attachment]");
        }
    }

    newItem->setData(msgPreview, Qt::ToolTipRole);

    itemModel->appendRow(newItem);
}

void SmsConversationsWidget::onConversationUpdated(qint64 conversationId, int newMsgIndex)
{
    QAbstractProxyModel *proxyModel = qobject_cast<QAbstractProxyModel *>(
        ui->conversationListView->model());
    QStandardItemModel *itemModel = qobject_cast<QStandardItemModel *>(proxyModel->sourceModel());

    SmsListItemData::Ptr correspondingItemData;
    QStandardItem *correspondingItem = nullptr;
    int rowCount = itemModel->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        correspondingItem = itemModel->item(i);
        SmsListItemData::Ptr itemData = qvariant_cast<SmsListItemData::Ptr>(
            correspondingItem->data(SmsListItemDataRoles::Data));

        Q_ASSERT(itemData);

        if (itemData->conversationId == conversationId) {
            correspondingItemData = itemData;
            break;
        }
    }

    if (!correspondingItemData) {
        return;
    }

    if (newMsgIndex == 0) {
        auto &conversationMsg = m_conversations[conversationId].at(newMsgIndex);

        QString senderName;
        // If the message is incoming, the sender is the first Address]
        if (conversationMsg.isOutgoing()) {
            senderName = tr("You");
        } else {
            auto phoneNumber = SMSHelper::canonicalizePhoneNumber(
                conversationMsg.addresses().at(0).address());
            auto contact = m_contactProvider->lookupContactByPhoneNumberOrName(phoneNumber);
            if (!contact.isEmpty()) {
                senderName = contact.realName();
            } else {
                senderName = phoneNumber;
            }
        }

        QString msgPreview = QLatin1StringView("%1: %2").arg(senderName).arg(conversationMsg.body());
        if (!conversationMsg.attachments().isEmpty()) {
            if (!conversationMsg.body().isEmpty()) {
                msgPreview = msgPreview + QStringLiteral(" ") + tr("[Attachment]");
            } else {
                msgPreview = msgPreview + tr("[Attachment]");
            }
        }
        correspondingItem->setData(msgPreview, Qt::ToolTipRole);

        correspondingItemData->latestMsgTime = conversationMsg.date();
    }

    if (correspondingItemData->conversationContentWidgetIndex > 0) {
        SmsConversationContentWidget *contentWidget = qobject_cast<SmsConversationContentWidget *>(
            ui->conversationContentWidgets->widget(
                correspondingItemData->conversationContentWidgetIndex));
        Q_ASSERT(contentWidget != nullptr);
        if (contentWidget != nullptr) {
            contentWidget->addMessage(newMsgIndex, m_conversations[conversationId].at(newMsgIndex));
        }
    }

    auto &conversationMsg = m_conversations[conversationId].at(newMsgIndex);
    if (!conversationMsg.attachments().isEmpty()) {
        Attachment att = conversationMsg.attachments().at(0);
        m_smsPluginWrapper->requestAttachment(att.partID(), att.uniqueIdentifier());
    }
}

void SmsConversationsWidget::uiUpdateConversationsContactInfo()
{
    QAbstractProxyModel *proxyModel = qobject_cast<QAbstractProxyModel *>(
        ui->conversationListView->model());
    QStandardItemModel *itemModel = qobject_cast<QStandardItemModel *>(proxyModel->sourceModel());
    int rowCount = itemModel->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QStandardItem *listItem = itemModel->item(i);
        SmsListItemData::Ptr itemData = qvariant_cast<SmsListItemData::Ptr>(
            listItem->data(SmsListItemDataRoles::Data));

        Q_ASSERT(itemData);

        QList<QPixmap> avatars;
        auto defaultAvatar = QIcon::fromTheme(QStringLiteral("im-user")).pixmap(1024, 1024);
        QStringList contactNameAndNumberList;
        for (int i = 0; i < itemData->canonicalizedPhoneNumbers.count(); ++i) {
            auto contact = m_contactProvider->lookupContactByPhoneNumberOrName(
                itemData->canonicalizedPhoneNumbers[i]);
            if (!contact.isEmpty()) {
                contactNameAndNumberList.push_back(contact.realName() + QLatin1StringView("(")
                                                   + itemData->canonicalizedPhoneNumbers[i]
                                                   + QLatin1StringView(")"));

                if (contact.photo().isEmpty()) {
                    avatars.append(defaultAvatar);
                } else {
                    avatars.append(QPixmap::fromImage(contact.photo().data()));
                }

            } else {
                contactNameAndNumberList.push_back(itemData->canonicalizedPhoneNumbers[i]);

                avatars.append(defaultAvatar);
            }
        }

        listItem->setData(ImageUtil::combineImage(avatars).scaled(g_listItemIconWidth,
                                                                  g_listItemIconWidth,
                                                                  Qt::KeepAspectRatio,
                                                                  Qt::SmoothTransformation),
                          Qt::DecorationRole);

        listItem->setData(contactNameAndNumberList.join(QLatin1StringView(", ")), Qt::DisplayRole);

        if (itemData->conversationId > -1) {
            QString senderName;
            ConversationMessage &conversationMsg = m_conversations[itemData->conversationId].first();
            // If the message is incoming, the sender is the first Address
            if (conversationMsg.isOutgoing()) {
                senderName = tr("You");
            } else {
                QList<ConversationAddress> conversationLists = conversationMsg.addresses();
                auto phoneNumber = SMSHelper::canonicalizePhoneNumber(
                    conversationLists[0].address());
                auto contact = m_contactProvider->lookupContactByPhoneNumberOrName(phoneNumber);
                if (!contact.isEmpty()) {
                    senderName = contact.realName();
                } else {
                    senderName = phoneNumber;
                }
            }

            QString msgPreview
                = QLatin1StringView("%1: %2").arg(senderName).arg(conversationMsg.body());
            if (!conversationMsg.attachments().isEmpty()) {
                if (!conversationMsg.body().isEmpty()) {
                    msgPreview = msgPreview + QStringLiteral(" ") + tr("[Attachment]");
                } else {
                    msgPreview = msgPreview + tr("[Attachment]");
                }
            }

            listItem->setData(msgPreview, Qt::ToolTipRole);
        }
    }
}
