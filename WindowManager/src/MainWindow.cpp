#include "MainWindow.hpp"
#include <QDebug>
#include "MyLogger.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_btnUpdate = std::make_unique<QPushButton>("Create New Window", this);
    m_btnUpdate->setGeometry(10, 10, 150, 30);
    setWindowTitle("Main Window");
    setGeometry(100, 100, 800, 600);
    connect(m_btnUpdate.get(), &QPushButton::clicked, this, &MainWindow::onUpdate);
    LOG_DEBUG << "MainWindow created";
}

MainWindow::~MainWindow()
{
    LOG_DEBUG << "~MainWindow called"; 
}

void MainWindow::onUpdate()
{
    LOG_DEBUG << "Update button clicked";
}