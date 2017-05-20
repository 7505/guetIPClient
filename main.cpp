#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowIcon(QIcon("./ico/icon.png"));
    w.setWindowTitle(QStringLiteral("桂林电子科技大学IP出校控制器v1.0"));
    w.show();


    return a.exec();
}
