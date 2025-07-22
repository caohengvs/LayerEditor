#pragma once
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QPointer>
#include <RotatingRectItem.hpp>
#include <unordered_map>
#include <variant>

class CustomScene final : public QGraphicsScene
{
    Q_OBJECT

    enum class ItemType
    {
        ImageItem,
        SelectRect,
        RotatingRectItem
    };

    using ItemMap = std::unordered_map<ItemType, std::variant<QGraphicsItem*, std::pair<QString, QGraphicsItem*>>>;

public:
    explicit CustomScene(QObject* parent = nullptr);
    explicit CustomScene(const QRectF& sceneRect, QObject* parent = nullptr);
    CustomScene(qreal x, qreal y, qreal width, qreal height, QObject* parent = nullptr);
    ~CustomScene() override = default;

public:
    bool loadImage(const QString& filePath);
    bool processImage();
    void showSelectRect(bool show = true);
    const QRectF getSelectRect();


private:
    ItemMap m_itemMap;
    RotatingRectItem* m_itemRotating;
};