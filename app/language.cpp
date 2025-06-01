#include "language.h"

#include "core/atexitmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QLatin1StringView>
#include <QLibraryInfo>
#include <QLocale>

Language::Language(QObject *parent)
    : QObject{parent}
{
    m_defaultLocaleName = QLatin1StringView("en_US");
    m_currentLocaleName = m_defaultLocaleName;

    m_localeNames.append(m_defaultLocaleName);
    m_localeNames.append(QLatin1StringView("zh_CN"));
}

bool Language::loadTranslation(const QString &localeName)
{
    if (m_currentLocaleName == localeName) {
        return true;
    }

    if (m_localeNames.indexOf(localeName) == -1) {
        return false;
    }

    unloadTranslation();

    if (m_defaultLocaleName == localeName) {
        return true;
    }

    QSharedPointer<QTranslator> qtTranslator(new QTranslator(this));
    QSharedPointer<QTranslator> appTranslator(new QTranslator(this));

    AtExitManager atExit;
    bool installSuccess = false;
    atExit.register_callback(make_closure([this, &qtTranslator, &appTranslator, &installSuccess]() {
        if (!installSuccess) {
            QCoreApplication::removeTranslator(appTranslator.get());
            QCoreApplication::removeTranslator(qtTranslator.get());
        }
    }));

    if (!qtTranslator->load(QLatin1StringView("qt_") + localeName,
                            QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        return false;
    }

    if (!QCoreApplication::installTranslator(qtTranslator.get())) {
        return false;
    }

    if (!appTranslator->load(QLatin1StringView("kdeconnect_") + localeName,
                             QCoreApplication::applicationDirPath() + QDir::separator()
                                 + QLatin1StringView("translations"))) {
        return false;
    }

    if (!QCoreApplication::installTranslator(appTranslator.get())) {
        return false;
    }

    installSuccess = true;
    m_qtTranslator = qtTranslator;
    m_appTranslator = appTranslator;
    m_currentLocaleName = localeName;
    QLocale::setDefault(QLocale(m_currentLocaleName));
    return installSuccess;
}

void Language::unloadTranslation()
{
    if (m_appTranslator) {
        QCoreApplication::removeTranslator(m_appTranslator.get());
        m_appTranslator.reset();
    }

    if (m_qtTranslator) {
        QCoreApplication::removeTranslator(m_qtTranslator.get());
        m_qtTranslator.reset();
    }

    m_currentLocaleName = m_defaultLocaleName;
    QLocale::setDefault(QLocale(m_currentLocaleName));
}
