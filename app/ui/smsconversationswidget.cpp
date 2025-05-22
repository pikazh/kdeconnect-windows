#include "smsconversationswidget.h"
#include "ui_smswidget.h"

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

SmsConversationsWidget::SmsConversationsWidget(Device::Ptr dev, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SMSWidget)
    , m_smsPluginWrapper(new SmsPluginWrapper(dev, this))
    , m_contactPluginWrapper(new ContactsPluginWrapper(dev, this))
{
    ui->setupUi(this);
    ui->addConversationButton->setIcon(RETRIEVE_THEME_ICON("list-add"));

    m_smsPluginWrapper->init();
    m_contactPluginWrapper->init();

    QObject::connect(m_smsPluginWrapper,
                     &SmsPluginWrapper::messagesReceived,
                     this,
                     &SmsConversationsWidget::onSmsMessagesReceived);

    m_smsPluginWrapper->requestAllConversations();

    QObject::connect(m_contactPluginWrapper,
                     &ContactsPluginWrapper::localCacheSynchronized,
                     this,
                     &SmsConversationsWidget::onContactUpdated);

    QObject::connect(m_contactPluginWrapper,
                     &ContactsPluginWrapper::localCacheRemoved,
                     this,
                     &SmsConversationsWidget::onContactDeleted);

    m_contacts = m_contactPluginWrapper->localCachedContacts();
}

SmsConversationsWidget::~SmsConversationsWidget()
{
    delete ui;
}

void SmsConversationsWidget::onSmsMessagesReceived(const QList<ConversationMessage> &msgList) {}

void SmsConversationsWidget::onContactUpdated(const QString &contactId,
                                              const KContacts::Addressee &address)
{
    m_contacts[contactId] = address;
}

void SmsConversationsWidget::onContactDeleted(const QList<QString> &contactIds)
{
    for (auto contactId : contactIds) {
        m_contacts.remove(contactId);
    }
}
