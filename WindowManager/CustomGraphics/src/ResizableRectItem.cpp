#include "ResizableRectItem.hpp"
#include <QCursor>
#include <QPainter>

ResizableRectItem::ResizableRectItem(const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setAcceptDrops(true);
}

void ResizableRectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // painter->setRenderHint(QPainter::Antialiasing, true);
    // 半透明填充
    QColor fillColor = Qt::cyan;
    fillColor.setAlpha(40);
    painter->setBrush(fillColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect());

    // 边框
    QPen pen(isSelected() ? Qt::red : Qt::blue, isSelected() ? 2 : 1);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect());

    // 手柄
    QColor handleColor = isSelected() ? Qt::red : Qt::blue;
    painter->setBrush(handleColor);
    painter->setPen(Qt::NoPen);
    for (int i = TopLeft; i <= BottomRight; ++i)
        painter->drawEllipse(handleRect(static_cast<HandleType>(i)));
}

ResizableRectItem::HandleType ResizableRectItem::handleAt(const QPointF& pos) const
{
    for (int i = TopLeft; i <= BottomRight; ++i)
        if (handleRect(static_cast<HandleType>(i)).contains(pos))
            return static_cast<HandleType>(i);
    return None;
}

QRectF ResizableRectItem::handleRect(HandleType handle) const
{
    QRectF r = rect();
    switch (handle)
    {
        case TopLeft:
            return QRectF(r.left() - m_handleSize / 2, r.top() - m_handleSize / 2, m_handleSize, m_handleSize);
        case TopRight:
            return QRectF(r.right() - m_handleSize / 2, r.top() - m_handleSize / 2, m_handleSize, m_handleSize);
        case BottomLeft:
            return QRectF(r.left() - m_handleSize / 2, r.bottom() - m_handleSize / 2, m_handleSize, m_handleSize);
        case BottomRight:
            return QRectF(r.right() - m_handleSize / 2, r.bottom() - m_handleSize / 2, m_handleSize, m_handleSize);
        default:
            return QRectF();
    }
}

void ResizableRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    HandleType handle = handleAt(event->pos());
    switch (handle)
    {
        case TopLeft:
        case BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case TopRight:
        case BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
    }
    QGraphicsRectItem::hoverMoveEvent(event);
}

void ResizableRectItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_handle = handleAt(event->pos());
    m_pressPos = event->pos();
    m_pressRect = rect();
    if (m_handle != None)
        event->accept();
    else
        QGraphicsRectItem::mousePressEvent(event);
}

void ResizableRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_handle == None)
    {
        QGraphicsRectItem::mouseMoveEvent(event);
        return;
    }
    QPointF delta = event->pos() - m_pressPos;
    QRectF r = m_pressRect;
    switch (m_handle)
    {
        case TopLeft:
            r.setTopLeft(r.topLeft() + delta);
            break;
        case TopRight:
            r.setTopRight(r.topRight() + delta);
            break;
        case BottomLeft:
            r.setBottomLeft(r.bottomLeft() + delta);
            break;
        case BottomRight:
            r.setBottomRight(r.bottomRight() + delta);
            break;
        default:
            break;
    }
    setRect(r.normalized());
}

void ResizableRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    m_handle = None;
    QGraphicsRectItem::mouseReleaseEvent(event);
}

QVariant ResizableRectItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    return QGraphicsRectItem::itemChange(change, value);
}

QRectF ResizableRectItem::boundingRect() const
{
    QRectF r = rect();
    qreal pad = m_handleSize;
    r.adjust(-pad, -pad, pad, pad);
    return r;
}
