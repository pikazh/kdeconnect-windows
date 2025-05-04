#pragma once

#include <QImage>
#include <QWidget>

class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = nullptr);

    bool loadImage(const QString &imagePath);
    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *paintEvent) override;
    virtual void resizeEvent(QResizeEvent *event) override;

    void generateScaledImage();

private:
    QImage m_image;
    QImage m_scaledImage;
};
