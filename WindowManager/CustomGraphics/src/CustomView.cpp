#include "CustomView.hpp"
#include <QDebug>
#include <QMimeData>
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
    auto* pScene = scene();
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

    event->accept();
}

void CustomView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    if (scene())
    {
        scene()->setSceneRect(QRectF(QPointF(0, 0), size()));
    }
}

void CustomView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().count() == 1)
    {
        event->acceptProposedAction();
    }
}

void CustomView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().count() == 1)
    {
        event->acceptProposedAction();
    }
}

void CustomView::dropEvent(QDropEvent* event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString filePath;

    filePath = urls.first().toLocalFile();

    emit filesDropped(filePath);

    event->acceptProposedAction();
}

void CustomView::init()
{
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("background-color: #333333;border: none;");
    setAcceptDrops(true);

}
