#include <singleapplication.h>

#include "application.h"

#include <QLocale>

int main(int argc, char *argv[])
{
    QLocale::setDefault(QLocale(QLocale::Language::English, QLocale::Territory::UnitedStates));

    Application app(argc, argv);
    SingleApplication single(argc, argv);
    if (!single.isPrimary()) {
        return 0;
    }

    QObject::connect(&single, &SingleApplication::instanceStarted, &app, [&app]() {
        app.showMainWindow();
    });

    app.init();
    app.showMainWindow();
    return app.exec();
}
