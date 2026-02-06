#include "../headers/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<std::shared_ptr<DownloadTask>>("std::shared_ptr<DownloadTask>");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QApplication::connect(&a, &QCoreApplication::aboutToQuit, [&w]() {
        //!!!
    });

    return a.exec();
}
