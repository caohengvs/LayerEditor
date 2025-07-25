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

    using ItemValue = std::variant<QGraphicsItem*, std::pair<QString, QGraphicsItem*>>;
    using ItemMap = std::unordered_map<ItemType, ItemValue>;

public:
    explicit CustomScene(QObject* parent = nullptr);
    explicit CustomScene(const QRectF& sceneRect, QObject* parent = nullptr);
    CustomScene(qreal x, qreal y, qreal width, qreal height, QObject* parent = nullptr);
    ~CustomScene() override = default;

public:
    bool loadImage(const QString& filePath);
    bool processImage();
    void showSelectRect(bool bShow = true);

private:
    const QRectF getSelectRect();
    void hide();
    void show(ItemType type, bool flag = true);
    void remove(ItemType type);
    const qreal topLevelZValue() const;
    void setTop(ItemType type);
    const QRectF getCenter(const QSizeF& size) const;
    bool hasItem(ItemType type) const;
    QGraphicsItem* getItem(ItemType type) const;
    void addItem(ItemType type, const ItemValue& val);

private:
    ItemMap m_itemMap;  // Map to store items by type
};