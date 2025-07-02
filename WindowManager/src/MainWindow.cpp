#include "MainWindow.hpp"
#include <QDebug>
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
       setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                   Qt::WindowCloseButtonHint);

    setGeometry(100, 100, 800, 600);
    setStyleSheet("background-color: #3f3f46");

    QWidget* centralWidget = new QWidget(this);
    m_customTitleBar = new CustomTitleBar(this, nullptr);
    CustomMenuBar* customMenuBar = new CustomMenuBar(centralWidget);

    auto& [scene, view] = m_mainSceneAndView;
    scene = new CustomScene(centralWidget);
    view = new CustomView(scene, centralWidget);
    view->setMinimumSize(width(), 400);

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

    assert(scene);
    mainLayout->addWidget(m_customTitleBar, mainLayout->rowCount(), 0);

    assert(customMenuBar);
    mainLayout->addWidget(customMenuBar, mainLayout->rowCount() + 1, 0);

    assert(view);
    mainLayout->addWidget(view, mainLayout->rowCount() + 1, 0);
    for (auto& itView : previewViews)
    {
        itView = new CustomView(previewScene, centralWidget);
        assert(itView);
        itView->setMinimumSize(80, 80);
        previewLayout->addWidget(itView);
    }
    previewScene->clearDefaultText();

    mainLayout->addLayout(previewLayout, mainLayout->rowCount() + 1, 0);
    mainLayout->setRowStretch(1, 1);

    setCentralWidget(centralWidget);
}
