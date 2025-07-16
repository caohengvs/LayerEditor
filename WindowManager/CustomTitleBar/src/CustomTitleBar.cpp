#include "CustomTitleBar.hpp"
#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QStyle>
#include <QTime>
#include <QTimer>

CustomTitleBar::CustomTitleBar(QMainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , m_parentWindow(mainWindow)
{
    setFixedHeight(30);
    setAttribute(Qt::WA_StyledBackground, true);

    m_closeButton = new QPushButton();
    QLabel* iconLabel = new QLabel(this);
    QLabel* titleLabel = new QLabel(QApplication::applicationName(), this);
    QLabel* timeLabel = new QLabel(QTime::currentTime().toString("hh:mm:ss"), this);
    QPixmap pixmap(":/icons/app.svg");
    iconLabel->setPixmap(pixmap.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);
    auto setupButton = [](QPushButton* btn, const QString& strPath)
    {
        int w = 30, h = 30;
        btn->setFixedSize(w, h);
        QIcon tmpIcon(strPath);
        btn->setIcon(tmpIcon);
        btn->setIconSize(QSize(w * 2 / 3, h * 2 / 3));
        btn->setFlat(true);
    };

    setupButton(m_closeButton, ":/icons/close.svg");
    m_closeButton->setObjectName("closeButton");
    connect(m_closeButton, &QPushButton::clicked, this, &CustomTitleBar::onCloseClicked);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->setSpacing(0);

    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addStretch();

    layout->addWidget(timeLabel);
    layout->addStretch();

    layout->addWidget(m_closeButton);

    setLayout(layout);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this,
            [timeLabel]() { timeLabel->setText(QTime::currentTime().toString("hh:mm:ss")); });
    m_timer->start(1000);

    setStyleSheet(styleSheet() + "QPushButton#closeButton:hover { background-color: red; }");
}

CustomTitleBar::~CustomTitleBar()
{
    qDebug() << "CustomTitleBar destructor called";
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