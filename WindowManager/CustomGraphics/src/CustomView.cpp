#include "CustomView.hpp"
#include <QDebug>
#include <QOpenGLWidget>
#include <QWheelEvent>

CustomView::CustomView(QWidget* parent)
    : QGraphicsView(parent)
{
    init();
}

CustomView::CustomView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    init();
}

CustomView::~CustomView()
{
}

void CustomView::wheelEvent(QWheelEvent* event)
{
    auto pScene = scene();
    if (!pScene)
    {
        event->accept();
        return;
    }

    if (pScene->items().isEmpty())
    {
        qWarning() << "CustomView::wheelEvent: No items in the scene to zoom.";
        event->accept();
        return;
    }
    
    QPointF scenePos = mapToScene(event->pos());

    constexpr double scaleFactor = 1.15;

    if (event->angleDelta().y() > 0)
    {
        scale(scaleFactor, scaleFactor);
    }
    else
    {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }

    QPointF newScenePos = mapToScene(event->pos());
    QPointF delta = newScenePos - scenePos;
    translate(delta.x(), delta.y());
    event->accept();
}

void CustomView::init()
{
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("background-color: #333333;border: none;");
}
