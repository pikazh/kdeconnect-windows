#include "smsconversationcontentwidget.h"
#include "app_debug.h"
#include "smscontentitem.h"
#include "smsitemdata.h"
#include "ui_smsconversationcontentwidget.h"

#include <QIcon>
#include <QListWidgetItem>
#include <QScrollBar>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

using namespace std::chrono_literals;

SmsConversationContentWidget::SmsConversationContentWidget(QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::SmsConversationContentWidget)
    , m_lastCheckMsgTime(std::chrono::system_clock::now())
    , m_checkMsgDuration(2000ms)
{
    ui->setupUi(this);
    ui->attachmentList->hide();
    ui->splitter->setCollapsible(0, false);

    auto font = ui->title->font();
    font.setPointSize(font.pointSize() + 2);
    ui->title->setFont(font);

    ui->listItemWidget->installEventFilter(this);
    ui->listItemWidget->sortItems(Qt::AscendingOrder);

    auto scrollBar = ui->listItemWidget->verticalScrollBar();
    scrollBar->installEventFilter(this);
    QObject::connect(scrollBar,
                     &QScrollBar::valueChanged,
                     this,
                     &SmsConversationContentWidget::onContentListVerticalScrollbarValueChanged);
}

SmsConversationContentWidget::~SmsConversationContentWidget()
{
    delete ui;
}

void SmsConversationContentWidget::setTitle(const QString &title)
{
    ui->title->setText(title);
}

void SmsConversationContentWidget::addMessage(int index, const ConversationMessage &message)
{
    QListWidgetItem *item = new QListWidgetItem(ui->listItemWidget);
    SmsContentItem *contentItemWidget = new SmsContentItem(ui->listItemWidget);
    item->setSizeHint(QSize(ui->listItemWidget->width(), contentItemWidget->size().height()));
    item->setText(QString::number(message.date()));

    ui->listItemWidget->addItem(item);
    ui->listItemWidget->setItemWidget(item, contentItemWidget);

    contentItemWidget->setText(message.body());
    if (message.isOutgoing()) {
        contentItemWidget->setName(tr("You"));
    } else {
        contentItemWidget->setName(message.addresses().at(0).address());
    }

    contentItemWidget->setTime(message.date());
    contentItemWidget->setAvatar(RETRIEVE_THEME_ICON("im-user").pixmap(32, 32));

    ui->listItemWidget->scrollToBottom();

    m_checkMsgDuration = 2000ms;
}

void SmsConversationContentWidget::adjustItemsSize()
{
    for (int i = 0; i < ui->listItemWidget->count(); ++i) {
        auto item = ui->listItemWidget->item(i);
        auto itemWidget = ui->listItemWidget->itemWidget(item);
        auto width = ui->listItemWidget->width() - ui->listItemWidget->verticalScrollBar()->width();
        item->setSizeHint(QSize(width, itemWidget->size().height()));
    }
}

bool SmsConversationContentWidget::eventFilter(QObject *obj, QEvent *event)
{
    auto evtType = event->type();
    if (obj == ui->listItemWidget) {
        if (evtType == QEvent::Resize || evtType == QEvent::Show)
            adjustItemsSize();
    } else if (obj == ui->listItemWidget->verticalScrollBar()) {
        if (evtType == QEvent::Hide || evtType == QEvent::Show)
            adjustItemsSize();
    }

    return QWidget::eventFilter(obj, event);
}

void SmsConversationContentWidget::onContentListVerticalScrollbarValueChanged(int value)
{
    if (value == 0) {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> dur = now - m_lastCheckMsgTime;
        if (dur > m_checkMsgDuration) {
            m_lastCheckMsgTime = now;
            m_checkMsgDuration = m_checkMsgDuration * 2;
            if (m_checkMsgDuration > 10000ms)
                m_checkMsgDuration = 10000ms;

            Q_EMIT requestMoreMessages();
        }
    }
}
