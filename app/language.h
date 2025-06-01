#pragma once

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTranslator>

class Language : public QObject
{
    Q_OBJECT
public:
    explicit Language(QObject *parent = nullptr);

    bool loadTranslation(const QString &localeName);
    void unloadTranslation();

    QString currentLocaleName() const { return m_currentLocaleName; }
    QStringList localeNames() const { return m_localeNames; }

private:
    QString m_defaultLocaleName;
    QString m_currentLocaleName;
    QList<QString> m_localeNames;

    QSharedPointer<QTranslator> m_qtTranslator;
    QSharedPointer<QTranslator> m_appTranslator;
};
