#include "CustomScene.hpp"
#include <ImageProcessor.hpp>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <ResizableRectItem.hpp>
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
    }

    remove(ItemType::ImageItem);  // Remove any existing ImageItem before loading a new one

    auto* pItem = new QGraphicsPixmapItem(QPixmap(filePath));
    if (pItem->pixmap().isNull())
    {
        qDebug() << "Failed to load image from" << filePath;
        return false;
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

    QPointF center = sceneRect().center();
    QSizeF size(50, 50);
    QRectF rect(center.x() - size.width() / 2, center.y() - size.height() / 2, size.width(), size.height());
    auto* pItem = new RotatingRectItem(rect);
    addItem(pItem);
    m_itemMap.insert_or_assign(ItemType::RotatingRectItem, pItem);

    pItem->startRotationAnimation(
        [this, rc, path]()
        {
            hide();
            show(ItemType::RotatingRectItem);
            qDebug() << "Rotation animation start.";
            ImageProcessor imageProcessor(path.toStdString());
            imageProcessor.processImageByAI({static_cast<int>(rc.x()), static_cast<int>(rc.y()),
                                             static_cast<int>(rc.width()), static_cast<int>(rc.height())});
        },
        1000);

    connect(
        pItem, &RotatingRectItem::finished, this,
        [this]()
        {
            qDebug() << "Rotation animation finished.";
            hide();
            show(ItemType::ImageItem);
            remove(ItemType::RotatingRectItem);
        },
        Qt::QueuedConnection);

    return true;
}

void CustomScene::showSelectRect(bool show)
{
    remove(ItemType::SelectRect);  // Ensure only one SelectRect exists at a time
    auto* pItem = new ResizableRectItem(QRectF(0, 0, 120, 50), nullptr);
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
    auto* selectRectItem = std::get<QGraphicsItem*>(itSelectFind->second);

    return QRectF(imageItem->mapFromItem(selectRectItem, QPointF(0, 0)), selectRectItem->boundingRect().size());
}

void CustomScene::hide()
{
    for (auto* pItem : items())
    {
        pItem->setVisible(false);
    }
}

void CustomScene::show(ItemType type)
{
    const auto& itFind = m_itemMap.find(type);
    if (itFind == m_itemMap.end())
    {
        qDebug() << "Item of type" << magic_enum::enum_name(type).data() << "not found in the scene.";
        return;
    }

    auto [key, val] = *itFind;
    switch (type)
    {
        case ItemType::ImageItem:
        {
            const auto& [path, item] = std::get<std::pair<QString, QGraphicsItem*>>(val);
            item->setVisible(true);
            break;
        }
        case ItemType::SelectRect:
            [[fallthrough]];
        case ItemType::RotatingRectItem:
        {
            auto* item = std::get<QGraphicsItem*>(val);
            item->setVisible(true);
            break;
        }
        default:
            break;
    }

    qDebug() << magic_enum::enum_name(type).data() << "is now visible.";
}

void CustomScene::remove(ItemType type)
{
    const auto& itFind = m_itemMap.find(type);
    if (itFind == m_itemMap.end())
    {
        qDebug() << "Item of type" << magic_enum::enum_name(type).data() << "not found in the scene.";
        return;
    }
    switch (type)
    {
        case ItemType::ImageItem:
        {
            const auto& [path, item] = std::get<std::pair<QString, QGraphicsItem*>>(itFind->second);
            removeItem(item);
            break;
        }
        case ItemType::SelectRect:
            [[fallthrough]];
        case ItemType::RotatingRectItem:
        {
            auto* item = std::get<QGraphicsItem*>(itFind->second);
            removeItem(item);
            qDebug() << "SelectRect item removed from the scene.";
            break;
        }

        default:
            break;
    }

    m_itemMap.erase(type);
    qDebug() << magic_enum::enum_name(type).data() << "removed from the scene.";
}
