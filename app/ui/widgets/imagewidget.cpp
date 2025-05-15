#include "imagewidget.h"
#include "app_debug.h"

#include <QBuffer>
#include <QImageReader>
#include <QPainter>
#include <QResizeEvent>

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget{parent}
{}

bool ImageWidget::loadImage(const QByteArray &imageData)
{
    if (imageData.isEmpty()) {
        m_image = QImage();
        return false;
    }

    QBuffer buffer;
    buffer.setData(imageData);
    QImageReader reader(&buffer);
    reader.setAutoTransform(true);

    if (!reader.canRead()) {
        qCWarning(KDECONNECT_APP) << "can't load image";
        m_image = QImage();
        return false;
    }

    if (!reader.read(&m_image)) {
        qCWarning(KDECONNECT_APP) << "corrupted image: " << reader.errorString();
        m_image = QImage();
        return false;
    }

    generateScaledImage();

    return true;
}

QSize ImageWidget::sizeHint() const
{
    if (m_image.isNull()) {
        return QSize(360, 360);
    } else {
        return m_image.size();
    }
}

void ImageWidget::paintEvent(QPaintEvent *paintEvent)
{
    if (m_image.isNull()) {
        return;
    }

    QPainter p(this);
    qreal iw = m_image.width();
    qreal ih = m_image.height();
    qreal ww = width();
    qreal wh = height();

    if (iw > ww || ih > wh) {
        iw = m_scaledImage.width();
        ih = m_scaledImage.height();
        p.translate(ww / 2, wh / 2);
        p.translate(-iw / 2, -ih / 2);
        p.drawImage(0, 0, m_scaledImage);
    } else {
        p.translate(ww / 2, wh / 2);
        p.translate(-iw / 2, -ih / 2);
        p.drawImage(0, 0, m_image);
    }
}

void ImageWidget::resizeEvent(QResizeEvent *event)
{
    if (m_image.isNull()) {
        return;
    }

    generateScaledImage();
}

void ImageWidget::generateScaledImage()
{
    qreal iw = m_image.width();
    qreal ih = m_image.height();
    qreal ww = width();
    qreal wh = height();

    if (iw > ww || ih > wh) {
        m_scaledImage = m_image.scaled(size(), Qt::KeepAspectRatio);
    }
}
