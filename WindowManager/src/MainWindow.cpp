#include "MainWindow.hpp"
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include "CustomFileListWidget.hpp"
#include "CustomMenuBar.hpp"
#include "CustomScene.hpp"
#include "CustomTitleBar.hpp"
#include "CustomView.hpp"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_customTitleBar(nullptr)
    , m_btnUpdate(nullptr)
    , m_mainSceneAndView(nullptr, nullptr)
    , m_previewSceneAndView(nullptr, std::array<CustomView*, 8>())
{
    init();
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow destructor called";
}

void MainWindow::init()
{

    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    int nH = 1000, nW = 1200;

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                   Qt::WindowCloseButtonHint);
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - nW) / 2;
    int y = (screenGeometry.height() - nH) / 2;
    setGeometry(x, y, nW,  nH);
    setStyleSheet("background-color: #3f3f46");

    QWidget* centralWidget = new QWidget(this);
    m_customTitleBar = new CustomTitleBar(this, nullptr);
    CustomMenuBar* customMenuBar = new CustomMenuBar(centralWidget);
    m_customFileList = new CustomFileListWidget(centralWidget);

    auto& [scene, view] = m_mainSceneAndView;
    scene = new CustomScene(centralWidget);
    view = new CustomView(scene, centralWidget);

    m_customFileList->setFixedWidth(200);
    view->setMinimumSize(QSize(width() / 2, 400));

    auto& [previewScene, previewViews] = m_previewSceneAndView;

    previewScene = new CustomScene(centralWidget);
    previewScene->setDefaultText("预览区域");

    QGridLayout* mainLayout = new QGridLayout(centralWidget);
    QHBoxLayout* previewLayout = new QHBoxLayout(nullptr);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setVerticalSpacing(1);

    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(0);

    scene->setDefaultText("画布区域");
    auto rowCount = mainLayout->rowCount();
    rowCount = mainLayout->rowCount();
    mainLayout->addWidget(m_customTitleBar, rowCount, 0, 1, 2);

    rowCount = mainLayout->rowCount();
    mainLayout->addWidget(customMenuBar, rowCount + 1, 0, 1, 2);

    rowCount = mainLayout->rowCount();
    mainLayout->addWidget(m_customFileList, rowCount + 1, 0);
    mainLayout->addWidget(view, rowCount + 1, 1);
    for (auto& itView : previewViews)
    {
        itView = new CustomView(previewScene, centralWidget);
        itView->setFixedHeight(200);

        previewLayout->addWidget(itView);
    }
    previewScene->clearDefaultText();

    rowCount = mainLayout->rowCount();
    mainLayout->addLayout(previewLayout, rowCount + 1, 0, 1, 2);

    setCentralWidget(centralWidget);

    connect(m_customFileList, &CustomFileListWidget::fileDoubleClicked, this,
            [](const auto& filePath) { qDebug() << "current selected file:" << filePath; });
}
