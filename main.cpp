#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <iostream>
#include "Logger.hpp"
#include "MainWindow.hpp"

int main(int argc, char** argv)
{
    auto& loggger = Logger::getInstance();
    loggger.init("MyApplication", Logger::DEBUG_L, true, false, "logs/app.log");

    QApplication app(argc, argv);

    app.setApplicationName("LayerEditor");
    qInstallMessageHandler(
        [](QtMsgType type, const QMessageLogContext& context, const QString& msg)
        {
            auto& logger = Logger::getInstance();

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

    qInfo() << "Application Name:" << app.applicationName();

    QIcon appIcon(":/icons/app.svg");
    app.setWindowIcon(appIcon);
    QFile styleFile(":/styles/qss/dark_theme.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream ts(&styleFile);
        QString styleSheet = ts.readAll();
        app.setStyleSheet(styleSheet);
        styleFile.close();
    }
    else
    {
        qWarning("Could not open stylesheet file: %s", qPrintable(styleFile.fileName()));
    }

    MainWindow* mainWindow = nullptr;
    mainWindow = new MainWindow();
    mainWindow->show();
    QObject::connect(&app, &QApplication::aboutToQuit,
                     [&]()
                     {
                         qInfo() << "Application is about to quit.";
                         delete mainWindow;
                         Logger::deleteInstance();
                     });

    return app.exec();
}
