#include "CustomScene.hpp"
#include <QDebug>

CustomScene::CustomScene(QObject* parent)
    : QGraphicsScene(parent)
{
    qDebug() << "CustomScene created with default constructor";
    initDefaultText("Welcome to Custom Scene!");
}

CustomScene::CustomScene(const QRectF& sceneRect, QObject* parent)
    : QGraphicsScene(sceneRect, parent)
{
    qDebug() << "CustomScene created with QRectF constructor";
    initDefaultText("Welcome to Custom Scene!");
}

CustomScene::CustomScene(qreal x, qreal y, qreal width, qreal height, QObject* parent)
    : QGraphicsScene(x, y, width, height, parent)
{
    qDebug() << "CustomScene created with x, y, width, height constructor";
    initDefaultText("Welcome to Custom Scene!");
}

void CustomScene::setDefaultText(const QString& text)
{
    if (!m_defaultTextItem)
        return;

    m_defaultTextItem->setPlainText(text);
    QRectF bounds = m_defaultTextItem->boundingRect();
    m_defaultTextItem->setPos(sceneRect().center() - bounds.center());
}

void CustomScene::clearDefaultText()
{
    if (!m_defaultTextItem)
        return;
        
    removeItem(m_defaultTextItem);
    delete m_defaultTextItem;
    m_defaultTextItem = nullptr;
}

void CustomScene::initDefaultText(const QString& text)
{
    if (m_defaultTextItem)
    {
        removeItem(m_defaultTextItem);
        delete m_defaultTextItem;
        m_defaultTextItem = nullptr;
    }
    m_defaultTextItem = new QGraphicsTextItem(text);
    m_defaultTextItem->setDefaultTextColor(Qt::gray);
    QFont font = m_defaultTextItem->font();
    font.setPointSize(24);
    font.setBold(true);
    m_defaultTextItem->setFont(font);

    addItem(m_defaultTextItem);

    QRectF bounds = m_defaultTextItem->boundingRect();
    m_defaultTextItem->setPos(sceneRect().center() - bounds.center());
}