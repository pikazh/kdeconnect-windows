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
    , m_smsManager(new SmsManager(dev, this))
    , m_contactProvider(new ContactProvider(dev, this))
    , m_delayFilterTimer(new QTimer(this))
{
    ui->setupUi(this);
    ui->addConversationButton->setIcon(RETRIEVE_THEME_ICON("list-add"));
    // ui->conversationContentWidgets->addWidget(
    //     new SmsConversationContentWidget(m_smsManager,
    //                                      -1,
    //                                      m_contactProvider,
    //                                      ui->conversationContentWidgets));
    // ui->conversationContentWidgets->setCurrentIndex(0);

    SmsListItemSortFilterProxyModel *proxyModel = new SmsListItemSortFilterProxyModel(
        ui->conversationListView);
    proxyModel->setSourceModel(new QStandardItemModel(ui->conversationListView));
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRole(Qt::DisplayRole);
    ui->conversationListView->setModel(proxyModel);
    ui->conversationListView->setItemDelegate(
        new SMSConversationListItemDelegate(ui->conversationListView));

    m_delayFilterTimer->setInterval(200);
    m_delayFilterTimer->setSingleShot(true);

    QObject::connect(m_delayFilterTimer, &QTimer::timeout, this, [this, proxyModel]() {
        proxyModel->setFilterFixedString(ui->conversationFilterEdit->text().trimmed());
    });

    QObject::connect(ui->conversationListView->selectionModel(),
                     &QItemSelectionModel::currentChanged,
                     this,
                     &SmsConversationsWidget::onConversationListSelectionChanged);

    QObject::connect(m_contactProvider,
                     &ContactProvider::contactUpdated,
                     this,
                     &SmsConversationsWidget::onContactUpdated);

    QObject::connect(m_smsManager,
                     &SmsManager::conversationStarted,
                     this,
                     &SmsConversationsWidget::onSmsConversationStarted);

    QObject::connect(m_smsManager,
                     &SmsManager::conversationNewMessage,
                     this,
                     &SmsConversationsWidget::onSmsConversationNewMessage);

    m_contactProvider->synchronize();
    m_smsManager->init();
}

SmsConversationsWidget::~SmsConversationsWidget()
{
    delete ui;
}

void SmsConversationsWidget::refreshConversation()
{
    // QAbstractProxyModel *proxyModel = qobject_cast<QAbstractProxyModel *>(
    //     ui->conversationListView->model());
    // QStandardItemModel *itemModel = qobject_cast<QStandardItemModel *>(proxyModel->sourceModel());
    // itemModel->clear();

    // ui->conversationContentWidgets->setCurrentIndex(0);
    // QList<QWidget *> contentWidgets;
    // for (int i = 1; i < ui->conversationContentWidgets->count(); ++i) {
    //     contentWidgets.push_back(ui->conversationContentWidgets->widget(i));
    // }
    // for (int i = 0; i < contentWidgets.size(); ++i) {
    //     auto w = contentWidgets.at(i);
    //     ui->conversationContentWidgets->removeWidget(w);
    //     w->deleteLater();
    // }

    m_contactProvider->synchronize();
    m_smsManager->refreshMessages();
}

void SmsConversationsWidget::on_conversationFilterEdit_textChanged(const QString &text)
{
    m_delayFilterTimer->start();
}

void SmsConversationsWidget::onSmsConversationStarted(const qint64 conversationId,
                                                      const ConversationMessage &msg)
{
    uiCreateConversationFromMessage(conversationId, msg);
}

void SmsConversationsWidget::onSmsConversationNewMessage(const qint64 conversationId,
                                                         const ConversationMessage &msg,
                                                         const int insertedIndex)
{
    uiUpdateConversationFromNewMessage(conversationId, msg, insertedIndex);
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
            SmsConversationContentWidget *contentWidget
                = new SmsConversationContentWidget(m_smsManager,
                                                   itemData->conversationId,
                                                   itemData->canonicalizedPhoneNumbers,
                                                   itemData->simcardSubId,
                                                   m_contactProvider,
                                                   ui->conversationContentWidgets);
            itemData->conversationContentWidgetIndex = ui->conversationContentWidgets->addWidget(
                contentWidget);
            ui->conversationContentWidgets->setCurrentWidget(contentWidget);

            contentWidget->setTitle(
                ui->conversationListView->model()->data(current, Qt::DisplayRole).toString());
        }
    }
}

void SmsConversationsWidget::uiCreateConversationFromMessage(const qint64 conversationId,
                                                             const ConversationMessage &msg)
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

    SmsListItemData::Ptr itemData(new SmsListItemData);
    itemData->conversationId = conversationId;
    itemData->latestMsgTime = msg.date();
    itemData->simcardSubId = msg.subID();
    QList<ConversationAddress> addresses = msg.addresses();
    for (int i = 0; i < addresses.count(); ++i) {
        itemData->canonicalizedPhoneNumbers.append(
            SMSHelper::canonicalizePhoneNumber(addresses[i].address()));
    }

    newItem->setData(QVariant::fromValue(itemData), SmsListItemDataRoles::Data);
    newItem->setData(QSize(0, g_listItemHeight), Qt::SizeHintRole);

    QList<QPixmap> avatars;
    auto defaultAvatar = QIcon::fromTheme(QStringLiteral("im-user")).pixmap(1024, 1024);
    QStringList contactNameAndNumberList;
    for (int i = 0; i < itemData->canonicalizedPhoneNumbers.count(); ++i) {
        auto contact = m_contactProvider->lookupContactByPhoneNumber(
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

    newItem->setData(ImageUtil::combineIcon(avatars).scaled(g_listItemIconWidth,
                                                            g_listItemIconWidth,
                                                            Qt::KeepAspectRatio,
                                                            Qt::SmoothTransformation),
                     Qt::DecorationRole);

    newItem->setData(contactNameAndNumberList.join(QLatin1StringView(", ")), Qt::DisplayRole);

    QString senderName;
    // If the message is incoming, the sender is the first Address]
    if (msg.isOutgoing()) {
        senderName = tr("You");
    } else {
        auto phoneNumber = SMSHelper::canonicalizePhoneNumber(addresses[0].address());
        auto contact = m_contactProvider->lookupContactByPhoneNumber(phoneNumber);
        if (!contact.isEmpty()) {
            senderName = contact.realName();
        } else {
            senderName = phoneNumber;
        }
    }

    QString msgPreview = QLatin1StringView("%1: %2").arg(senderName).arg(msg.body());

    if (!msg.attachments().isEmpty()) {
        if (!msg.body().isEmpty()) {
            msgPreview = msgPreview + QStringLiteral(" ") + tr("[Attachment]");
        } else {
            msgPreview = msgPreview + tr("[Attachment]");
        }
    }

    newItem->setData(msgPreview, Qt::ToolTipRole);

    itemModel->appendRow(newItem);
}

void SmsConversationsWidget::uiUpdateConversationFromNewMessage(const qint64 conversationId,
                                                                const ConversationMessage &msg,
                                                                const int insertedIndex)
{
    if (insertedIndex == 0) {
        QAbstractProxyModel *proxyModel = qobject_cast<QAbstractProxyModel *>(
            ui->conversationListView->model());
        QStandardItemModel *itemModel = qobject_cast<QStandardItemModel *>(
            proxyModel->sourceModel());

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

        QString senderName;
        // If the message is incoming, the sender is the first Address]
        if (msg.isOutgoing()) {
            senderName = tr("You");
        } else {
            auto phoneNumber = SMSHelper::canonicalizePhoneNumber(msg.addresses().at(0).address());
            auto contact = m_contactProvider->lookupContactByPhoneNumber(phoneNumber);
            if (!contact.isEmpty()) {
                senderName = contact.realName();
            } else {
                senderName = phoneNumber;
            }
        }

        QString msgPreview = QLatin1StringView("%1: %2").arg(senderName).arg(msg.body());
        if (!msg.attachments().isEmpty()) {
            if (!msg.body().isEmpty()) {
                msgPreview = msgPreview + QStringLiteral(" ") + tr("[Attachment]");
            } else {
                msgPreview = msgPreview + tr("[Attachment]");
            }
        }
        correspondingItem->setData(msgPreview, Qt::ToolTipRole);

        correspondingItemData->latestMsgTime = msg.date();
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
            auto contact = m_contactProvider->lookupContactByPhoneNumber(
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

        listItem->setData(ImageUtil::combineIcon(avatars).scaled(g_listItemIconWidth,
                                                                 g_listItemIconWidth,
                                                                 Qt::KeepAspectRatio,
                                                                 Qt::SmoothTransformation),
                          Qt::DecorationRole);

        QString title = contactNameAndNumberList.join(QLatin1StringView(", "));
        listItem->setData(title, Qt::DisplayRole);
        if (itemData->conversationContentWidgetIndex > -1) {
            SmsConversationContentWidget *widget = qobject_cast<SmsConversationContentWidget *>(
                ui->conversationContentWidgets->widget(itemData->conversationContentWidgetIndex));

            widget->setTitle(title);
        }

        if (itemData->conversationId > -1) {
            QString senderName;
            const auto [hasConversation, msgList] = m_smsManager->conversationMessages(
                itemData->conversationId);
            Q_ASSERT(hasConversation);
            if (hasConversation) {
                const ConversationMessage &conversationMsg = msgList.first();
                // If the message is incoming, the sender is the first Address
                if (conversationMsg.isOutgoing()) {
                    senderName = tr("You");
                } else {
                    QList<ConversationAddress> conversationLists = conversationMsg.addresses();
                    auto phoneNumber = SMSHelper::canonicalizePhoneNumber(
                        conversationLists[0].address());
                    auto contact = m_contactProvider->lookupContactByPhoneNumber(phoneNumber);
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
}
