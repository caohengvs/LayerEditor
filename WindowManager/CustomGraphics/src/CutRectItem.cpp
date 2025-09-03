#include "CutRectItem.hpp"
#include <QMouseEvent>
#include <QPainter>


CutRectItem::CutRectItem(QGraphicsRectItem* parent)
    : QGraphicsRectItem(parent)
{
    setMouseTracking(true);
}

void CutRectItem::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));

    // 绘制十字
    painter.drawLine(0, m_startPoint.y(), width(), m_startPoint.y());
    painter.drawLine(m_startPoint.x(), 0, m_startPoint.x(), height());

    // 如果正在绘制，则画出矩形框
    if (m_isDrawing)
    {
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        QRect rect(m_startPoint, m_endPoint);
        painter.drawRect(rect.normalized());
    }
}

void CutRectItem::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_startPoint = event->pos();
        m_isDrawing = true;
        // 触发重绘，以便画出十字
        update();
    }
}

void CutRectItem::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDrawing)
    {
        m_endPoint = event->pos();
        // 触发重绘，以便画出拖动的矩形
        update();
    }
    else
    {
        // 如果没有按键，也可以更新十字的位置
        m_startPoint = event->pos();
        update();
    }
}

void CutRectItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDrawing = false;
        // 在这里你可以处理最终的矩形框
        qDebug() << "Selected rectangle: " << QRect(m_startPoint, m_endPoint).normalized();
    }
}