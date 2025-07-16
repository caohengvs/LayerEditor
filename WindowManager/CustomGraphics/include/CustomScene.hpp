#pragma once
#include <QGraphicsScene>
#include <QGraphicsTextItem>
class CustomScene final : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit CustomScene(QObject* parent = nullptr);
    explicit CustomScene(const QRectF& sceneRect, QObject* parent = nullptr);
    CustomScene(qreal x, qreal y, qreal width, qreal height, QObject* parent = nullptr);
    ~CustomScene() override = default;

public:
    void setDefaultText(const QString& text);
    void clearDefaultText();
    bool loadImage(const QString& filePath);

private:
    void initDefaultText(const QString& text);

private:
    QGraphicsTextItem* m_defaultTextItem = nullptr; 
};