#include "../headers/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QApplication::connect(&a, &QCoreApplication::aboutToQuit, [&w]() {
        //!!!
    });

    return a.exec();
}
