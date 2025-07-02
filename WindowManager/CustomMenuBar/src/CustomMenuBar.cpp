#include "CustomMenuBar.hpp"
#include <QDebug>
#include <QJsonObject>

CustomMenuBar::CustomMenuBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(50);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "CustomMenuBar {"
        "   background-color: #27272a; /* zinc-800 */"
        // "   border-bottom: 1px solid #3f3f46; /* zinc-700 */"
        // "   border-radius: 6px; /* rounded-md */"
        "   padding: 0 16px; "
        "}"
        "QPushButton {"
        "   background-color: transparent; "
        "   color: #d4d4d8; "
        "   border: none; "
        "   padding: 4px 12px; "
        "   border-radius: 4px; "
        "   font-size: 13px; "
        "   font-weight: 500; "
        "}"
        "QPushButton:hover {"
        "   background-color: #3f3f46; /* zinc-700 */"
        "   color: #ffffff; /* hover:text-white */"
        "}"
        "QPushButton:pressed {"
        "   background-color: #52525b;"
        "}");

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(1);

    QPushButton* importBtn = new QPushButton;
    QPushButton* removeBtn = new QPushButton;
    QPushButton* playAndPauseBtn = new QPushButton;
    auto setupButton = [](QPushButton* btn, const QString& strPath)
    {
        QIcon icon(strPath);
        btn->setIcon(icon);
        btn->setIconSize(QSize(28, 28));
        btn->setFlat(true);
    };

    setupButton(importBtn, ":/icons/import.svg");
    setupButton(playAndPauseBtn, ":/icons/play.svg");
    setupButton(removeBtn, ":/icons/remove.svg");

    m_mainLayout->addWidget(importBtn);
    m_mainLayout->addWidget(playAndPauseBtn);
    m_mainLayout->addWidget(removeBtn);

    m_mainLayout->addStretch();

    connect(importBtn, &QPushButton::clicked, this, &CustomMenuBar::onImportButtonClicked);
    connect(playAndPauseBtn, &QPushButton::clicked, this, &CustomMenuBar::onPlayAndPauseButtonClicked);
    connect(removeBtn, &QPushButton::clicked, this, &CustomMenuBar::onRemoveButtonClicked);
}

CustomMenuBar::~CustomMenuBar()
{
}

void CustomMenuBar::onImportButtonClicked()
{
    emit importClicked();
}

void CustomMenuBar::onPlayAndPauseButtonClicked()
{
    emit playAndPauseClicked();
}

void CustomMenuBar::onRemoveButtonClicked()
{
    emit removeClicked();
}