#include "notifybysnore.h"
#include "notification.h"
#include "notification_debug.h"
#include "snoretoastactions.h"

#include <QGuiApplication>
#include <QIcon>
#include <QLocalSocket>

NotifyBySnore::NotifyBySnore(QObject *parent)
    : NotificationPlugin(parent)
{
    m_server.listen(QString::number(qHash(QCoreApplication::applicationDirPath())));

    connect(&m_server, &QLocalServer::newConnection, this, [this]() {
        QLocalSocket *responseSocket = m_server.nextPendingConnection();
        connect(responseSocket, &QLocalSocket::readyRead, [this, responseSocket]() {
            const QByteArray rawNotificationResponse = responseSocket->readAll();
            responseSocket->deleteLater();

            const QString notificationResponse
                = QString::fromWCharArray(reinterpret_cast<const wchar_t *>(
                                              rawNotificationResponse.constData()),
                                          rawNotificationResponse.size() / sizeof(wchar_t));
            qDebug(NOTIFICATIONS_MOD) << notificationResponse;

            QMap<QString, QStringView> notificationResponseMap;
            for (const auto str :
                 QStringView(notificationResponse).split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
                const int equalIndex = str.indexOf(QLatin1Char('='));
                notificationResponseMap.insert(str.sliced(0, equalIndex).toString(),
                                               str.sliced(equalIndex + 1));
            }

            const QString responseAction = notificationResponseMap[QStringLiteral("action")]
                                               .toString();
            const int responseNotificationId
                = notificationResponseMap[QStringLiteral("notificationId")].toInt();

            qCDebug(NOTIFICATIONS_MOD) << "The notification ID is : " << responseNotificationId;

            auto it = m_notifications.find(responseNotificationId);
            if (it == m_notifications.end()) {
                qCWarning(NOTIFICATIONS_MOD) << "Received a response for an unknown notification.";
                return;
            }

            Notification *n = it.value();

            switch (SnoreToastActions::getAction(responseAction.toStdWString())) {
            case SnoreToastActions::Actions::Clicked:
                qCDebug(NOTIFICATIONS_MOD) << "User clicked on the toast.";
                break;

            case SnoreToastActions::Actions::Hidden:
                qCDebug(NOTIFICATIONS_MOD) << "The toast got hidden.";
                break;

            case SnoreToastActions::Actions::Dismissed:
                qCDebug(NOTIFICATIONS_MOD) << "User dismissed the toast.";
                break;

            case SnoreToastActions::Actions::Timedout:
                qCDebug(NOTIFICATIONS_MOD) << "The toast timed out.";
                break;

            case SnoreToastActions::Actions::ButtonClicked: {
                qCDebug(NOTIFICATIONS_MOD) << "User clicked an action button in the toast.";
                const QString responseButton = notificationResponseMap[QStringLiteral("button")]
                                                   .toString();
                Q_EMIT actionInvoked(n, responseButton);
                break;
            }

            case SnoreToastActions::Actions::TextEntered: {
                qCDebug(NOTIFICATIONS_MOD) << "User entered some text in the toast.";
                const QString replyText = notificationResponseMap[QStringLiteral("text")].toString();
                qCDebug(NOTIFICATIONS_MOD) << "Text entered was :: " << replyText;
                //Q_EMIT replied(responseNotificationId, replyText);
                break;
            }

            default:
                qCWarning(NOTIFICATIONS_MOD) << "Unexpected behaviour with the toast. Please file "
                                                "a bug report / feature request.";
                break;
            }

            // Action Center callbacks are not yet supported so just close the notification once done
            this->close(n);
        });
    });
}

NotifyBySnore::~NotifyBySnore()
{
    m_server.close();
}

void NotifyBySnore::init()
{
    installAppShortCut();
}

void NotifyBySnore::notify(Notification *notification)
{
    QMetaObject::invokeMethod(this,
                              &NotifyBySnore::notifyDeferred,
                              Qt::QueuedConnection,
                              notification);
}

void NotifyBySnore::close(Notification *notification)
{
    qCDebug(NOTIFICATIONS_MOD) << "Requested to close notification with ID:" << notification->id();

    m_notifications.remove(notification->id());
    const QStringList snoretoastArgsList{QStringLiteral("-close"), QString::number(notification->id()), QStringLiteral("-appID"), QCoreApplication::instance()->applicationName()};

    //qCDebug(NOTIFICATIONS_MOD) << "Closing notification; SnoreToast process arguments:" << snoretoastArgsList;
    QProcess::startDetached(SnoreToastExecPath(), snoretoastArgsList);

    Q_EMIT finished(notification);
}

void NotifyBySnore::notifyDeferred(Notification *notification)
{
    m_notifications[notification->id()] = notification;

    const QString notificationTitle = ((!notification->title().isEmpty())
                                           ? notification->title()
                                           : QCoreApplication::instance()->applicationName());
    QStringList snoretoastArgsList{QStringLiteral("-id"),
                                   QString::number(notification->id()),
                                   QStringLiteral("-t"),
                                   notificationTitle,
                                   QStringLiteral("-m"),
                                   stripRichText(notification->text()),
                                   QStringLiteral("-appID"),
                                   QCoreApplication::instance()->applicationName(),
                                   QStringLiteral("-pipename"),
                                   m_server.fullServerName()};

    // handle the icon for toast notification
    QString iconPath = m_iconDir.path() + QDir::separator() + QString::number(notification->id()) + QLatin1Char('_');
    bool hasIcon = false;
    if(!notification->pixmap().isNull())
    {
        iconPath += QString::number(notification->pixmap().cacheKey());
        hasIcon = notification->pixmap().save(iconPath, "PNG");
    } else if (!notification->iconName().isEmpty()) {
        iconPath += notification->iconName();
        QIcon icon = QIcon::fromTheme(notification->iconName());
        if (!icon.isNull()) {
            hasIcon = icon.pixmap(64, 64).save(iconPath, "PNG");
        }
    } else {
        iconPath += QStringLiteral("app");
        QGuiApplication *guiApp = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
        if(guiApp != nullptr)
        {
            hasIcon = guiApp->windowIcon().pixmap(1024, 1024).save(iconPath, "PNG");
        }
    }

    if (hasIcon)
    {
        snoretoastArgsList << QStringLiteral("-p") << iconPath;
    }

    // if'd below, because SnoreToast currently doesn't support both textbox and buttons in the same notification
    /*if (notification->replyAction())
    {
        snoretoastArgsList << QStringLiteral("-tb");
    }
    else */if (!notification->actions().isEmpty())
    {
        // add actions if any
        const auto actions = notification->actions();
        QStringList actionLabels;
        for (NotificationAction *action : actions)
        {
            action->setId(action->text());
            actionLabels << action->text();
        }

        snoretoastArgsList << QStringLiteral("-b") << actionLabels.join(QLatin1Char(';'));
    }

    QProcess *snoretoastProcess = new QProcess();
    connect(snoretoastProcess,
            &QProcess::readyReadStandardError,
            this,
            [snoretoastProcess, snoretoastArgsList]() {
                const auto data = snoretoastProcess->readAllStandardError();
                qCDebug(NOTIFICATIONS_MOD)
                    << "SnoreToast process stderr:" << snoretoastArgsList << data;
            });
    connect(snoretoastProcess,
            &QProcess::readyReadStandardOutput,
            this,
            [snoretoastProcess, snoretoastArgsList]() {
                const auto data = snoretoastProcess->readAllStandardOutput();
                qCDebug(NOTIFICATIONS_MOD)
                    << "SnoreToast process stdout:" << snoretoastArgsList << data;
            });
    connect(snoretoastProcess,
            &QProcess::errorOccurred,
            this,
            [snoretoastProcess, snoretoastArgsList, iconPath](QProcess::ProcessError error) {
                qCWarning(NOTIFICATIONS_MOD)
                    << "SnoreToast process errored:" << snoretoastArgsList << error;
                snoretoastProcess->deleteLater();
                QFile::remove(iconPath);
            });
    connect(snoretoastProcess,
            &QProcess::finished,
            this,
            [snoretoastProcess, snoretoastArgsList, iconPath](int exitCode,
                                                              QProcess::ExitStatus exitStatus) {
                qCDebug(NOTIFICATIONS_MOD) << "SnoreToast process finished:" << snoretoastArgsList;
                qCDebug(NOTIFICATIONS_MOD) << "code:" << exitCode << "status:" << exitStatus;
                snoretoastProcess->deleteLater();
                QFile::remove(iconPath);
            });

    qCDebug(NOTIFICATIONS_MOD) << "SnoreToast process starting:" << snoretoastArgsList;
    snoretoastProcess->start(SnoreToastExecPath(), snoretoastArgsList);
}

QString NotifyBySnore::SnoreToastExecPath()
{
    return QCoreApplication::applicationDirPath() + QDir::separator()
           + QStringLiteral("snoretoast.exe");
}

void NotifyBySnore::installAppShortCut()
{
    const QString appId = QCoreApplication::applicationName();
    QProcess proc;
    proc.start(SnoreToastExecPath(),
               {QStringLiteral("-install"), appId, QCoreApplication::applicationFilePath(), appId});
    proc.waitForFinished();
}
