#include "CustomScene.hpp"

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
    adjustImg(pItem);
    addItem(ItemType::ImageItem, std::pair<QString, QGraphicsItem*>(filePath, pItem));
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

    auto* pItem = new RotatingRectItem(getCenter(QSizeF(50, 50)));
    addItem(ItemType::RotatingRectItem, pItem);
    std::optional<ImageProcessor::STImageInfo> stInfo = std::nullopt;
    pItem->startRotationAnimation(
        [this, rc, path, &stInfo]()
        {
            hide();
            show(ItemType::RotatingRectItem);
            qDebug() << "Rotation animation start.";

            ImageProcessor imageProcessor(path.toStdString());
            imageProcessor.processImageByAI({static_cast<int>(rc.x()), static_cast<int>(rc.y()),
                                             static_cast<int>(rc.width()), static_cast<int>(rc.height())});

            stInfo = imageProcessor.getImageInfo();
        },
        1000);

    connect(
        pItem, &RotatingRectItem::finished, this,
        [this,&stInfo]()
        {
            qDebug() << "Rotation animation finished.";
            hide();
            show(ItemType::SelectRect);
            if (stInfo != std::nullopt)
            {
                QImage image(reinterpret_cast<uchar*>(stInfo.value().buffer.get()), stInfo.value().cols,
                             stInfo.value().rows, stInfo.value().step, QImage::Format_RGB888);

                QPixmap pixmap = QPixmap::fromImage(image);
                auto* pItem = new QGraphicsPixmapItem(pixmap);
                adjustImg(pItem);
                addItem(ItemType::OutImageItem, pItem);
            }

            remove(ItemType::RotatingRectItem);
        },
        Qt::QueuedConnection);

    return true;
}

void CustomScene::showSelectRect(bool bShow)
{
    if (!hasItem(ItemType::SelectRect))
    {
        auto* pItem = new ResizableRectItem(getCenter(QSize(120, 50)), nullptr);
        addItem(ItemType::SelectRect, pItem);
        setTop(ItemType::SelectRect);
        return;
    }

    show(ItemType::SelectRect, bShow);
    setTop(ItemType::SelectRect);
}

void CustomScene::showOriginalImg(const bool visible)
{
    show(ItemType::ImageItem, visible);
    show(ItemType::OutImageItem, !visible);
}

const QRectF CustomScene::getSelectRect()
{
    auto* pSelectItem = getItem(ItemType::SelectRect);
    auto* pImgItem = getItem(ItemType::ImageItem);

    if (pSelectItem == nullptr || pImgItem == nullptr)
    {
        qDebug() << "SelectRect or ImageItem not found in the scene.";
        return QRectF();
    }

    // 由于是在缩放后的图像上进行选择，所以需要将在缩放后的选择矩形转换回原图像的坐标系
    qreal inverseScale = 1 / pImgItem->scale();

    return QRectF(pImgItem->mapFromItem(pSelectItem, pSelectItem->boundingRect().topLeft()),
                  pSelectItem->boundingRect().size() * inverseScale);
}

void CustomScene::hide()
{
    for (auto* pItem : items())
    {
        pItem->setVisible(false);
    }
}

void CustomScene::show(ItemType type, bool bFlag)
{
    auto* pItem = getItem(type);
    if (!pItem)
    {
        return;
    }

    pItem->setVisible(bFlag);
    qDebug() << magic_enum::enum_name(type).data() << "is now" << (bFlag ? "visible." : "hidden.");
}

void CustomScene::remove(ItemType type)
{
    auto* pItem = getItem(type);
    if (!pItem)
    {
        return;
    }

    removeItem(pItem);

    m_itemMap.erase(type);

    qDebug() << magic_enum::enum_name(type).data() << "removed from the scene.";
}

const qreal CustomScene::topLevelZValue() const
{
    qreal maxZ = 0;
    for (auto* item : items())
    {
        maxZ = std::max(maxZ, item->zValue());
    }
    return maxZ;
}

void CustomScene::setTop(ItemType type)
{
    auto* pItem = getItem(type);
    if (!pItem)
    {
        qDebug() << "Item of type" << magic_enum::enum_name(type).data() << "not found in the scene.";
        return;
    }

    qreal zVal = topLevelZValue() + 1;

    pItem->setZValue(zVal);
}

const QRectF CustomScene::getCenter(const QSizeF& size) const
{
    QPointF center = sceneRect().center();
    QRectF rect(center.x() - size.width() / 2, center.y() - size.height() / 2, size.width(), size.height());
    return rect;
}

bool CustomScene::hasItem(ItemType type) const
{
    return m_itemMap.find(type) != m_itemMap.end();
}

QGraphicsItem* CustomScene::getItem(ItemType type) const
{
    QGraphicsItem* itemToModify = nullptr;
    const auto& itFind = m_itemMap.find(type);
    if (itFind == m_itemMap.end())
    {
        qDebug() << "Item of type" << magic_enum::enum_name(type).data() << "not found in the scene.";
        return itemToModify;
    }

    std::visit(
        [&itemToModify, type](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::pair<QString, QGraphicsItem*>>)
            {
                itemToModify = arg.second;
            }
            else if constexpr (std::is_same_v<T, QGraphicsItem*>)
            {
                itemToModify = arg;
            }
            else
            {
                qDebug() << "not found the type in m_itemMap:" << magic_enum::enum_name(type).data();
                return;
            }
        },
        itFind->second);

    return itemToModify;
}

void CustomScene::addItem(ItemType type, const ItemValue& val)
{
    std::visit(
        [this, type](auto&& arg)
        {
            QGraphicsItem* itemToAdd = nullptr;
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, QGraphicsItem*>)
            {
                itemToAdd = arg;
            }
            else if constexpr (std::is_same_v<T, std::pair<QString, QGraphicsItem*>>)
            {
                itemToAdd = arg.second;
            }
            QGraphicsScene::addItem(itemToAdd);
        },
        val);

    m_itemMap.insert_or_assign(type, val);
}

void CustomScene::adjustImg(QGraphicsPixmapItem* item)
{
    QRectF sceneRect = this->sceneRect();
    QPixmap pix = item->pixmap();
    qreal scaleX = sceneRect.width() / pix.width();
    qreal scaleY = sceneRect.height() / pix.height();
    qreal scale = qMin(scaleX, scaleY);
    item->setScale(scale);
    qreal x = sceneRect.center().x() - (pix.width() * scale) / 2.0;
    qreal y = sceneRect.center().y() - (pix.height() * scale) / 2.0;
    item->setPos(x, y);
}
