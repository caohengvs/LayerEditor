#include "CustomScene.hpp"
#include <ImageProcessor.hpp>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <magic_enum/magic_enum.hpp>

CustomScene::CustomScene(QObject* parent)
    : QGraphicsScene(parent)
    , m_itemRotating(nullptr)
{
}

CustomScene::CustomScene(const QRectF& sceneRect, QObject* parent)
    : QGraphicsScene(sceneRect, parent)
    , m_itemRotating(nullptr)
{
}

CustomScene::CustomScene(qreal x, qreal y, qreal width, qreal height, QObject* parent)
    : QGraphicsScene(x, y, width, height, parent)
    , m_itemRotating(nullptr)
{
}

bool CustomScene::loadImage(const QString& filePath)
{
    auto pItem = new QGraphicsPixmapItem(QPixmap(filePath));
    if (pItem->pixmap().isNull())
    {
        qDebug() << "Failed to load image from" << filePath;
        return false;
    }

    pItem->setTransformationMode(Qt::SmoothTransformation);
    addItem(pItem);
    m_itemMap.insert_or_assign(ItemType::ImageItem, pItem);
    qDebug() << magic_enum::enum_name(ItemType::ImageItem).data() << "loaded successfully from" << filePath;
    m_imagePath = filePath;

    return true;
}

bool CustomScene::processImage()
{
    if (m_imagePath.isEmpty())
    {
        qDebug() << "No image loaded to process.";
        return false;
    }
    const auto& rc = getSelectRect();

    auto pItem = new RotatingRectItem(QRectF(sceneRect().center(), QSizeF(50, 50)));
    addItem(pItem);
    m_itemMap.insert_or_assign(ItemType::RotatingRectItem, pItem);

    pItem->startRotationAnimation(
        [this, rc]()
        {
            m_itemMap[ItemType::ImageItem]->setVisible(false);
            ImageProcessor imageProcessor(getImagePath().toStdString());
            imageProcessor.processImage({static_cast<int>(rc.x()), static_cast<int>(rc.y()),
                                         static_cast<int>(rc.width()), static_cast<int>(rc.height())});
        },
        1000);

    connect(pItem, &RotatingRectItem::finished, this,
            [this]()
            {
                qDebug() << "Rotation animation finished.";
                m_itemMap[ItemType::ImageItem]->setVisible(true);
                auto item = m_itemMap[ItemType::RotatingRectItem];
                removeItem(item);
                m_itemMap.erase(ItemType::RotatingRectItem);
            });

    return true;
}

void CustomScene::showSelectRect(bool show)
{
    auto pItem = new QGraphicsRectItem(QRectF(0, 0, 120, 50), nullptr);
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
    auto point = m_itemMap[ItemType::ImageItem]->mapFromItem(m_itemMap[ItemType::SelectRect], QPointF(0, 0));
    auto size = m_itemMap[ItemType::SelectRect]->boundingRect().size();

    return QRectF(point, size);
}
