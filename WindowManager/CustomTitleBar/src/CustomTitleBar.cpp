#include "CustomTitleBar.hpp"
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include "MyLogger.hpp"

CustomTitleBar::CustomTitleBar(QMainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , m_parentWindow(mainWindow)
{
    setFixedHeight(30);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #333333;");

    m_closeButton = new QPushButton();
    QLabel* iconLabel = new QLabel(this);
    QPixmap pixmap(":/icons/app.svg");
    iconLabel->setPixmap(pixmap.scaled(30,30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);
    auto setupButton = [](QPushButton* btn, const QString& strPath)
    {
        int w = 32, h = 32;
        btn->setFixedSize(w, h);
        QIcon tmpIcon(strPath);
        btn->setIcon(tmpIcon);
        btn->setIconSize(QSize(w, h));
        btn->setFlat(true);
        btn->setStyleSheet(
            "QPushButton { border: none; background-color: transparent;}"
            "QPushButton:hover { background-color: #555555; }"
            "QPushButton#closeButton:hover { background-color: red; }");
    };

    setupButton(m_closeButton, ":/icons/close.svg");
    m_closeButton->setObjectName("closeButton");

    connect(m_closeButton, &QPushButton::clicked, this, &CustomTitleBar::onCloseClicked);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->setSpacing(0);

    layout->addWidget(iconLabel);
    layout->addStretch();

    layout->addWidget(m_closeButton);

    setLayout(layout);
}

CustomTitleBar::~CustomTitleBar()
{
    LOG_DEBUG << "CustomTitleBar destructor called";
}

void CustomTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (m_parentWindow && event->button() == Qt::LeftButton)
    {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - m_parentWindow->frameGeometry().topLeft();
        event->accept();
    }
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (m_parentWindow && m_isDragging && event->buttons() & Qt::LeftButton)
    {
        m_parentWindow->move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    m_isDragging = false;
    event->accept();
}

void CustomTitleBar::onCloseClicked()
{
    QApplication::quit();
}