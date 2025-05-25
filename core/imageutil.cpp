#include "imageutil.h"

#include <QPainter>

QPixmap ImageUtil::combineImage(const QList<QPixmap> &images)
{
    QPixmap retImage;
    if (images.size() == 1) {
        return images.first();
    } else {
        // Cook an icon by combining the available icons
        // Barring better information, use the size of the first icon as the size for the final icon
        QSize size = images.first().size();
        QPixmap canvas(size);
        canvas.fill(Qt::transparent);
        QPainter painter(&canvas);

        QSize halfSize = size / 2;

        QRect topLeftQuadrant(QPoint(0, 0), halfSize);
        QRect topRightQuadrant(topLeftQuadrant.topRight(), halfSize);
        QRect bottomLeftQuadrant(topLeftQuadrant.bottomLeft(), halfSize);
        QRect bottomRightQuadrant(topLeftQuadrant.bottomRight(), halfSize);

        if (images.size() == 2) {
            painter.drawPixmap(topLeftQuadrant, images[0]);
            painter.drawPixmap(bottomRightQuadrant, images[1]);
        } else if (images.size() == 3) {
            QRect topMiddle(QPoint(halfSize.width() / 2, 0), halfSize);
            painter.drawPixmap(topMiddle, images[0]);
            painter.drawPixmap(bottomLeftQuadrant, images[1]);
            painter.drawPixmap(bottomRightQuadrant, images[2]);
        } else {
            // Four or more
            painter.drawPixmap(topLeftQuadrant, images[0]);
            painter.drawPixmap(topRightQuadrant, images[1]);
            painter.drawPixmap(bottomLeftQuadrant, images[2]);
            painter.drawPixmap(bottomRightQuadrant, images[3]);
        }

        return canvas;
    }
}
