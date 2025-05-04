#include <QLocale>
#include <QTranslator>

#include "application.h"

int main(int argc, char *argv[])
{
    Application app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "XConnect_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    app.init();
    app.showMainWindow();
    return app.exec();
}
