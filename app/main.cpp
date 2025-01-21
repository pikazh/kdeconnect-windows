#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "Windows.h"
int main(int argc, char *argv[])
{
    ::MessageBox(0,0,0,0);
    QGuiApplication app(argc, argv);
    app.setOrganizationDomain("KdeConnect");
    app.setOrganizationName("zyx");
    app.setApplicationName("KdeConnect");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(QUrl("qrc:/qml/Main.qml"));
    return app.exec();
}
