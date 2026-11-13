#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("HexCompare"));
    app.setOrganizationName(QStringLiteral("HexCompare"));

    MainWindow window;
    window.show();
    return app.exec();
}