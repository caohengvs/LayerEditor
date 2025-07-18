#include "CustomScene.hpp"
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <magic_enum/magic_enum.hpp>

CustomScene::CustomScene(QObject* parent)
    : QGraphicsScene(parent)
{
}

CustomScene::CustomScene(const QRectF& sceneRect, QObject* parent)
    : QGraphicsScene(sceneRect, parent)
{
}

CustomScene::CustomScene(qreal x, qreal y, qreal width, qreal height, QObject* parent)
    : QGraphicsScene(x, y, width, height, parent)
{
}

bool CustomScene::loadImage(const QString& filePath)
{
    auto pItem = std::make_unique<QGraphicsPixmapItem>(QPixmap(filePath));
    if (pItem->pixmap().isNull())
    {
        qDebug() << "Failed to load image from" << filePath;
        return false;
    }

    pItem->setTransformationMode(Qt::SmoothTransformation);
    addItem(pItem.get());
    m_itemMap.insert_or_assign(ItemType::ImageItem, std::move(pItem));
    qDebug() << magic_enum::enum_name(ItemType::ImageItem).data() << "loaded successfully from" << filePath;
    m_imagePath = filePath;

    return true;
}

void CustomScene::showSelectRect(bool show)
{
    auto pItem = std::make_unique<QGraphicsRectItem>(QRectF(0, 0, 120, 50), nullptr);
    pItem->setPen(QPen(Qt::red, 2));
    pItem->setBrush(QBrush(Qt::transparent));
    pItem->setFlag(QGraphicsItem::ItemIsMovable);
    pItem->setFlag(QGraphicsItem::ItemIsSelectable);
    pItem->setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    addItem(pItem.get());

    m_itemMap.insert_or_assign(ItemType::SelectRect, std::move(pItem));
    connect(
        this, &QGraphicsScene::selectionChanged, this,
        [this]()
        {
         
        });
}

const QRectF CustomScene::getSelectRect()
{
    auto point = m_itemMap[ItemType::ImageItem]->mapFromItem(m_itemMap[ItemType::SelectRect].get(), QPointF(0, 0));
    auto size = m_itemMap[ItemType::SelectRect]->boundingRect().size();

    return QRectF(point, size);
}
