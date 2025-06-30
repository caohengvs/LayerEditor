#include <QApplication>
#include "TestWindow.hpp"
#include "logger/include/MyLogger.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    INIT_MY_LOGGER("MyApplication", MyLogger::DEBUG_L, true, "logs/app.log");
    LOG_INFO << "This is an info message";
    LOG_DEBUG << "debug";

    QApplication app(argc, argv);

    app.setApplicationName("MyApplication");

    MainWindow mainWindow;
    mainWindow.setWindowTitle("Test Window");
    mainWindow.setGeometry(100, 100, 800, 600);
    mainWindow.show();

    return app.exec();
}
