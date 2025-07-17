#pragma once
#include <QGraphicsScene>
#include <QKeyEvent>
#include <unordered_map>
#include <QGraphicsRectItem>

class CustomScene final : public QGraphicsScene
{
    Q_OBJECT

    enum class ItemType
    {
        ImageItem,
        SelectRect
    };

    template<typename T>
    using ItemMap = std::unordered_map<ItemType, std::unique_ptr<T>>;

public:
    explicit CustomScene(QObject* parent = nullptr);
    explicit CustomScene(const QRectF& sceneRect, QObject* parent = nullptr);
    CustomScene(qreal x, qreal y, qreal width, qreal height, QObject* parent = nullptr);
    ~CustomScene() override = default;

public:
    bool loadImage(const QString& filePath);
    void showSelectRect(bool show = true);

private:
    ItemMap<QGraphicsItem> m_itemMap;
};