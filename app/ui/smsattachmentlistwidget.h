#pragma once

#include <QResizeEvent>
#include <QSet>
#include <QStringList>
#include <QTreeWidget>

class SmsAttachmentListWidget : public QTreeWidget
{
    Q_OBJECT
public:
    SmsAttachmentListWidget(QWidget *parent = nullptr);

    void init();
    void addAttachmentList(const QStringList &list);

    bool isAttachmentListEmpty() const { return m_addedLowerCaseCanonicalizedFiles.isEmpty(); }
    QStringList attachmentList() const;
    void clearAttachmentList();

Q_SIGNALS:
    void attachmentAdded(const QString &path);
    void attachmentRemoved(const QString &path);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

    enum Columns {
        Path = 0,
        Button,
        Count,
    };

private:
    QSet<QString> m_addedLowerCaseCanonicalizedFiles;
};
