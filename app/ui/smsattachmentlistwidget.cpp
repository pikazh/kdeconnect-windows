#include "smsattachmentlistwidget.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QObject>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QVariant>

#define BUTTON_COLUMN_WIDTH (40)

SmsAttachmentListWidget::SmsAttachmentListWidget(QWidget *parent)
    : QTreeWidget(parent)
{}

void SmsAttachmentListWidget::init()
{
    this->setColumnCount(Columns::Count);
}

void SmsAttachmentListWidget::addAttachmentList(const QStringList &list)
{
    for (auto &filePath : list) {
        QFileInfo fileInfo(filePath);
        QString canonicalizedFilePath = fileInfo.canonicalFilePath();
        QString canonicalizedLowerCaseFilePath = canonicalizedFilePath.toLower();
        if (!canonicalizedLowerCaseFilePath.isEmpty()
            && !m_addedLowerCaseCanonicalizedFiles.contains(canonicalizedLowerCaseFilePath)) {
            m_addedLowerCaseCanonicalizedFiles.insert(canonicalizedLowerCaseFilePath);

            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(Columns::Path, canonicalizedFilePath);
            item->setToolTip(Columns::Path, canonicalizedFilePath);
            this->addTopLevelItem(item);

            QWidget *w = new QWidget(this);
            QHBoxLayout *layout = new QHBoxLayout(w);
            layout->setContentsMargins(0, 0, 0, 0);
            QPushButton *removeButton = new QPushButton(w);
            removeButton->setProperty("item", QVariant::fromValue(item));
            removeButton->setProperty("filePath", canonicalizedLowerCaseFilePath);
            removeButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
            removeButton->setToolTip(tr("Delete"));
            layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
            layout->addWidget(removeButton);
            layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
            w->setFixedWidth(BUTTON_COLUMN_WIDTH);
            this->setItemWidget(item, Columns::Button, w);

            QObject::connect(
                removeButton,
                &QPushButton::clicked,
                this,
                [this]() {
                    QObject *sender = QObject::sender();
                    QTreeWidgetItem *item = qvariant_cast<QTreeWidgetItem *>(
                        sender->property("item"));
                    QString filePath = qvariant_cast<QString>(sender->property("filePath"));
                    Q_ASSERT(item != nullptr && !filePath.isEmpty());
                    m_addedLowerCaseCanonicalizedFiles.remove(filePath);
                    QString canonicalizedFilePath = item->text(Columns::Path);
                    Q_ASSERT(!canonicalizedFilePath.isEmpty());
                    this->removeItemWidget(item, Columns::Button);
                    this->takeTopLevelItem(indexOfTopLevelItem(item));
                    delete item;

                    Q_EMIT attachmentRemoved(canonicalizedFilePath);
                },
                Qt::QueuedConnection);

            Q_EMIT attachmentAdded(canonicalizedFilePath);
        }
    }
}

QStringList SmsAttachmentListWidget::attachmentList() const
{
    QStringList ret;
    for (int i = 0; i < this->topLevelItemCount(); ++i) {
        QString path = this->topLevelItem(i)->text(Columns::Path);
        Q_ASSERT(!path.isEmpty());
        ret.append(path);
    }

    return ret;
}

void SmsAttachmentListWidget::clearAttachmentList()
{
    this->clear();
    m_addedLowerCaseCanonicalizedFiles.clear();
}

void SmsAttachmentListWidget::resizeEvent(QResizeEvent *event)
{
    this->header()->resizeSection(Columns::Path, this->width() - BUTTON_COLUMN_WIDTH);
    QTreeWidget::resizeEvent(event);
}

void SmsAttachmentListWidget::showEvent(QShowEvent *event)
{
    this->header()->resizeSection(Columns::Path, this->width() - BUTTON_COLUMN_WIDTH);
    QTreeWidget::showEvent(event);
}
