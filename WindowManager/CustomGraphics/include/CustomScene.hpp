#pragma once
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QPointer>
#include <RotatingRectItem.hpp>
#include <unordered_map>
#include <variant>
#include <optional>
#include <ImageProcessor.hpp>
class CustomView;

class CustomScene final : public QGraphicsScene
{
    friend class CustomView;
    Q_OBJECT

    enum class ItemType
    {
        ImageItem,
        OutImageItem,
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
    void showOriginalImg();

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
    void adjustImg(QGraphicsPixmapItem* item);

private:
    ItemMap m_itemMap;  // Map to store items by type
    std::optional<ImageProcessor::STImageInfo> stInfo = std::nullopt;
};