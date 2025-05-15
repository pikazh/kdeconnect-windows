/**
 * SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "presenterplugin.h"
#include "core/plugins/pluginfactory.h"
#include "plugin_presenter_debug.h"

#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QScreen>
#include <QWidget>

K_PLUGIN_CLASS_WITH_JSON(PresenterPlugin, "kdeconnect_presenter.json")

class PresenterView : public QWidget
{
    Q_OBJECT
public:
    PresenterView(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        m_yOffsetProp = m_xOffsetProp = 0.5f;

        Qt::WindowFlags windowFlags = Qt::WindowFlags(
            Qt::WindowDoesNotAcceptFocus | Qt::WindowFullScreen | Qt::WindowStaysOnTopHint
            | Qt::FramelessWindowHint | Qt::Tool);
#ifdef Q_OS_WIN
        windowFlags |= Qt::WindowTransparentForInput;
#endif
        setWindowFlags(windowFlags);
        setAttribute(Qt::WA_TranslucentBackground);
    }

    bool loadImage()
    {
        QImageReader reader(":/dot.png");
        reader.setAutoTransform(true);

        if (!reader.canRead()) {
            qCWarning(KDECONNECT_PLUGIN_PRESENTER) << "can't load image";
            m_image = QImage();
            return false;
        }

        if (!reader.read(&m_image)) {
            qCWarning(KDECONNECT_PLUGIN_PRESENTER) << "corrupted image: " << reader.errorString();
            m_image = QImage();
            return false;
        }

        QSize screenSize = screen()->size();
        int scaledImageWidth = screenSize.width() / 16;
        m_image = m_image.scaled(scaledImageWidth,
                                 scaledImageWidth,
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);

        return true;
    }

    void setOffset(qreal xOffsetProp, qreal yOffsetProp)
    {
        this->m_xOffsetProp = xOffsetProp;
        this->m_yOffsetProp = yOffsetProp;
        update();
    }

protected:
    virtual void paintEvent(QPaintEvent *event) override
    {
        if (m_image.isNull()) {
            return;
        }

        QPainter p(this);
        qreal iw = m_image.width();
        qreal ih = m_image.height();
        auto rect = frameGeometry();
        qreal xOffset = rect.width() * m_xOffsetProp;
        qreal yOffset = rect.height() * m_yOffsetProp;

        p.translate(xOffset - iw / 2, yOffset - ih / 2);
        p.drawImage(0, 0, m_image);
    }

private:
    QImage m_image;
    qreal m_xOffsetProp;
    qreal m_yOffsetProp;
};

PresenterPlugin::PresenterPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
    , m_view(nullptr)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(500);
    m_timer->setSingleShot(true);
}

void PresenterPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.get<bool>(QStringLiteral("stop"), false)) {
        if (m_view) {
            m_view->close();
            m_view.reset();
        }
        return;
    }

    if (!m_view) {
        m_xPos = 0.5;
        m_yPos = 0.5;
        m_view.reset(new PresenterView());
        m_view->loadImage();
        m_view->showFullScreen();
        connect(m_timer, &QTimer::timeout, this, [this]() {
            if (m_view) {
                m_view->close();
                m_view.reset();
            }
        });
    }

    QSize screenSize = m_view->screen()->size();
    qreal ratio = qreal(screenSize.width()) / screenSize.height();

    m_xPos += np.get<qreal>(QStringLiteral("dx"));
    m_yPos += np.get<qreal>(QStringLiteral("dy")) * ratio;
    m_xPos = qBound(0.0, m_xPos, 1.0);
    m_yPos = qBound(0.0, m_yPos, 1.0);
    m_view->setOffset(m_xPos, m_yPos);

    m_timer->start();
}

#include "presenterplugin.moc"
