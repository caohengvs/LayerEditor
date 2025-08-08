#include "MainWindow.hpp"
#include <ImageProcessor.hpp>
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
    setGeometry(x, y, nW, nH);
    setStyleSheet("background-color: #3f3f46");

    QWidget* centralWidget = new QWidget(this);
    m_customTitleBar = new CustomTitleBar(this, nullptr);
    CustomMenuBar* customMenuBar = new CustomMenuBar(centralWidget);
    auto& [scene, view] = m_mainSceneAndView;
    scene = new CustomScene(centralWidget);
    view = new CustomView(scene, centralWidget);

    view->setMinimumSize(QSize(width() / 2, 400));
    scene->setSceneRect(0, 0, view->width(), view->height());

    auto& [previewScene, previewViews] = m_previewSceneAndView;

    previewScene = new CustomScene(centralWidget);

    QGridLayout* mainLayout = new QGridLayout(centralWidget);
    QHBoxLayout* previewLayout = new QHBoxLayout(nullptr);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setVerticalSpacing(1);

    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(0);

    auto rowCount = mainLayout->rowCount();
    rowCount = mainLayout->rowCount();
    mainLayout->addWidget(m_customTitleBar, rowCount, 0, 1, 2);

    rowCount = mainLayout->rowCount();
    mainLayout->addWidget(customMenuBar, rowCount + 1, 0, 1, 2);

    rowCount = mainLayout->rowCount();
    mainLayout->addWidget(view, rowCount + 1, 0, 1, 2);

    for (auto& itView : previewViews)
    {
        itView = new CustomView(previewScene, centralWidget);
        itView->setFixedHeight(200);

        previewLayout->addWidget(itView);
    }

    rowCount = mainLayout->rowCount();
    mainLayout->addLayout(previewLayout, rowCount + 1, 0, 1, 2);

    setCentralWidget(centralWidget);

    connect(customMenuBar, &CustomMenuBar::removeClicked, this,
            [&scene]()
            {
                static bool bShow = true;
                scene->showSelectRect(bShow);
                bShow = !bShow;
            });

    connect(customMenuBar, &CustomMenuBar::doneClicked, this, [scene]() { 
        scene->processImage();
     });

    connect(customMenuBar, &CustomMenuBar::rotateClicked,
            [view, scene](const auto& val)
            {
                view->rotate(val);
            });

    connect(view, &CustomView::filesDropped, this, [scene](const auto& filePath) { scene->loadImage(filePath); });

     connect(customMenuBar, &CustomMenuBar::showOriginalImg, this, [scene]() { scene->showOriginalImg(); });
}
