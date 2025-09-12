#include "mainwindow.h"

#include <QApplication>
#include <future>
#include "PCIEWindow.hpp"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    MainWindow s(nullptr);
    s.show();

    return a.exec();
}
