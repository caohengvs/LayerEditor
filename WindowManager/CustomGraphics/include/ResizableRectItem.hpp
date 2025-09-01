#pragma once
#include <QCursor>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>

class ResizableRectItem : public QGraphicsRectItem
{
public:
    enum HandleType
    {
        None,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Move
    };
    ResizableRectItem(const QRectF& rect, QGraphicsItem* parent = nullptr);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    QRectF boundingRect() const override;

private:
    HandleType m_handle = None;
    QPointF m_pressPos;
    QRectF m_pressRect;
    qreal m_handleSize = 8.0;

    HandleType handleAt(const QPointF& pos) const;
    QRectF handleRect(HandleType handle) const;
};