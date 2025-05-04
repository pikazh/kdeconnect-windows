#pragma once

#include "task.h"

#include <QNetworkReply>
#include <QUrl>

class KDECONNECTCORE_EXPORT NetAction : public Task
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<NetAction>;

    NetAction(QObject *parent = nullptr);
    virtual ~NetAction() override = default;

    QUrl url() const { return m_url; }
    void setUrl(const QUrl &url) { m_url = url; }
    void setNetworkAccessManager(shared_qobject_ptr<QNetworkAccessManager> network)
    {
        m_network = network;
    }

    shared_qobject_ptr<QNetworkAccessManager> networkAccessManager() const { return m_network; }

protected:
    virtual void onAbort() override;
    virtual void executeTask() override;
    virtual QNetworkReply *createReply() = 0;

protected Q_SLOTS:
    void onNetworkReplyFinished();
    void onNetworkReplyError(QNetworkReply::NetworkError code);

private:
    QUrl m_url;
    shared_qobject_ptr<QNetworkAccessManager> m_network;
    shared_qobject_ptr<QNetworkReply> m_reply;
};
