#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class DesktopFile : public QObject
{
public:
    DesktopFile(const QString &filePath, QObject *parent = nullptr);
    virtual ~DesktopFile() override = default;

    QString readIcon();
    QString readName();

protected:
    QString translatedValue(QAnyStringView key, const QString &defValue = {});

private:
    QSettings m_settings;
};
