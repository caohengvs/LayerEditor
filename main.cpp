#include <QApplication>
#include <QDebug>
#include <iostream>
#include "MainWindow.hpp"
#include "MyLogger.hpp"

int main(int argc, char** argv)
{
    auto & loggger = MyLogger::getInstance();
    loggger.init("MyApplication", MyLogger::DEBUG_L, true, "logs/app.log",true);

    QApplication app(argc, argv);

    app.setApplicationName("LayerEditor");
    QIcon appIcon(":/icons/app.svg");
    app.setWindowIcon(appIcon);
    qInstallMessageHandler(
        [](QtMsgType type, const QMessageLogContext& context, const QString& msg)
        {
            auto& logger = MyLogger::getInstance();

            switch (type)
            {
                case QtDebugMsg:
                {
                    logger.debug(context.file, context.line, context.function) << msg.toStdString();
                    break;
                }
                case QtInfoMsg:
                {
                    logger.info(context.file, context.line, context.function) << msg.toStdString();
                    break;
                }
                case QtWarningMsg:
                {
                    logger.warn(context.file, context.line, context.function) << msg.toStdString();
                    break;
                }
                case QtCriticalMsg:
                {
                    logger.error(context.file, context.line, context.function) << msg.toStdString();
                    break;
                }
                case QtFatalMsg:
                    logger.critical(context.file, context.line, context.function) << msg.toStdString();
                    abort();
            }
        });

    MainWindow* mainWindow = nullptr;
    mainWindow = new MainWindow();
    mainWindow->show();
    QObject::connect(&app, &QApplication::aboutToQuit,
                     [&]()
                     {
                         qInfo() << "Application is about to quit.";
                         delete mainWindow;
                         MyLogger::deleteInstance();
                     });

    return app.exec();
}
