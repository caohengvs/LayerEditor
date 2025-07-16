#include "CustomScene.hpp"
#include <QDebug>

CustomScene::CustomScene(QObject* parent)
    : QGraphicsScene(parent)
{
    initDefaultText("Welcome to Custom Scene!");
}

CustomScene::CustomScene(const QRectF& sceneRect, QObject* parent)
    : QGraphicsScene(sceneRect, parent)
{
    initDefaultText("Welcome to Custom Scene!");
}

CustomScene::CustomScene(qreal x, qreal y, qreal width, qreal height, QObject* parent)
    : QGraphicsScene(x, y, width, height, parent)
{
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

bool CustomScene::loadImage(const QString& filePath)
{
    auto pItem = new QGraphicsPixmapItem(QPixmap(filePath));
    if (pItem->pixmap().isNull())
    {
        qDebug() << "Failed to load image from" << filePath;
        delete pItem;  
        return false;
    }

    pItem->setTransformationMode(Qt::SmoothTransformation);
    addItem(pItem);
    return true;
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