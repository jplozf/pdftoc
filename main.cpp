#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/16x16/pdf.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
