#include "CustomScene.hpp"
#include <ImageProcessor.hpp>
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
    auto* pItem = new QGraphicsPixmapItem(QPixmap(filePath));
    if (pItem->pixmap().isNull())
    {
        qDebug() << "Failed to load image from" << filePath;
        return false;
    }

    const auto& itFind = m_itemMap.find(ItemType::ImageItem);
    if (itFind != m_itemMap.end())
    {
        auto [key, val] = *itFind;
        auto [path, item] = std::get<std::pair<QString, QGraphicsItem*>>(val);
        if (path == filePath)
        {
            qDebug() << "Image already loaded from" << filePath;
            return false;
        }

        removeItem(item);
        m_itemMap.erase(ItemType::ImageItem);
    }

    pItem->setTransformationMode(Qt::SmoothTransformation);
    addItem(pItem);
    m_itemMap.insert_or_assign(ItemType::ImageItem, std::pair<QString, QGraphicsItem*>(filePath, pItem));
    qDebug() << magic_enum::enum_name(ItemType::ImageItem).data() << "loaded successfully from" << filePath;

    return true;
}

bool CustomScene::processImage()
{
    const auto& itImageFind = m_itemMap.find(ItemType::ImageItem);
    if (itImageFind == m_itemMap.end())
    {
        qDebug() << "No image item found in the scene.";
        return false;
    }
    const auto& [path, item] = std::get<std::pair<QString, QGraphicsItem*>>(itImageFind->second);

    const auto& rc = getSelectRect();

    auto* pItem = new RotatingRectItem(QRectF(sceneRect().center(), QSizeF(50, 50)));
    addItem(pItem);
    m_itemMap.insert_or_assign(ItemType::RotatingRectItem, pItem);

    pItem->startRotationAnimation(
        [this, rc, path, item]()
        {
            item->setVisible(false);
            ImageProcessor imageProcessor(path.toStdString());
            imageProcessor.processImageByAI({static_cast<int>(rc.x()), static_cast<int>(rc.y()),
                                         static_cast<int>(rc.width()), static_cast<int>(rc.height())});
        },
        1000);

    connect(
        pItem, &RotatingRectItem::finished, this,
        [this, itImageFind, path, item]()
        {
            qDebug() << "Rotation animation finished.";
            item->setVisible(true);
            auto* rectItem = std::get<QGraphicsItem*>(m_itemMap[ItemType::RotatingRectItem]);
            removeItem(rectItem);
            m_itemMap.erase(ItemType::RotatingRectItem);
        },
        Qt::QueuedConnection);

    return true;
}

void CustomScene::showSelectRect(bool show)
{
    const auto& itFind = m_itemMap.find(ItemType::SelectRect);
    if (itFind != m_itemMap.end())
    {
        auto [key, val] = *itFind;
        auto* item = std::get<QGraphicsItem*>(val);

        removeItem(item);
        m_itemMap.erase(ItemType::SelectRect);
    }
    auto* pItem = new QGraphicsRectItem(QRectF(0, 0, 120, 50), nullptr);
    pItem->setPen(QPen(Qt::red, 2));
    pItem->setBrush(QBrush(Qt::transparent));
    pItem->setFlag(QGraphicsItem::ItemIsMovable);
    pItem->setFlag(QGraphicsItem::ItemIsSelectable);
    pItem->setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    addItem(pItem);
    m_itemMap.insert_or_assign(ItemType::SelectRect, pItem);
}

const QRectF CustomScene::getSelectRect()
{
    const auto& itImageFind = m_itemMap.find(ItemType::ImageItem);
    const auto& itSelectFind = m_itemMap.find(ItemType::SelectRect);

    if (itImageFind == m_itemMap.end() || itSelectFind == m_itemMap.end())
    {
        qDebug() << "Image or SelectRect item not found in the scene.";
        return QRectF();
    }

    const auto& [path, imageItem] = std::get<std::pair<QString, QGraphicsItem*>>(itImageFind->second);
    const auto& selectRectItem = std::get<QGraphicsItem*>(itSelectFind->second);

    return QRectF(imageItem->mapFromItem(selectRectItem, QPointF(0, 0)), selectRectItem->boundingRect().size());
}