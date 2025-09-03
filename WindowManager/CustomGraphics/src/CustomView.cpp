#include "CustomView.hpp"
#include <CustomScene.hpp>
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsTextItem>
#include <QMimeData>
#include <QOpenGLWidget>
#include <QWheelEvent>

CustomView::CustomView(QWidget* parent)
    : QGraphicsView(parent)
    , m_showCut(false)
{
    init();
}

CustomView::CustomView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , m_showCut(false)
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
        setStyleSheet("background-color: #1f7394ff;border: none");
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

void CustomView::dragLeaveEvent(QDragLeaveEvent* event)
{
    setStyleSheet("background-color: #333333;border: none;");
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
    setStyleSheet("background-color: #333333;border: none;");
}

void CustomView::rotate(qreal angle)
{
    auto* pScene = dynamic_cast<CustomScene*>(scene());
    if (!pScene)
        return;

    auto* pItem = dynamic_cast<QGraphicsPixmapItem*>(pScene->getItem(CustomScene::ItemType::ImageItem));
    if (!pItem)
        return;

    QGraphicsView::rotate(angle);

    fitInView(pItem->sceneBoundingRect(), Qt::KeepAspectRatio);
}

void CustomView::showCut(const bool visible)
{
    setMouseTracking(visible);
    m_showCut = visible;
    // 如果之前有矩形，先移除
    if (m_rectItem)
    {
        scene()->removeItem(m_rectItem);
        delete m_rectItem;
        m_rectItem = nullptr;
    }
    // 创建一个新的 QGraphicsRectItem
    m_rectItem = new QGraphicsRectItem();
    m_rectItem->setPen(QPen(Qt::blue, 1, Qt::DashLine));
    m_rectItem->setBrush(QBrush(QColor(0, 0, 255, 50)));  // 半透明蓝色
    scene()->addItem(m_rectItem);
}

void CustomView::init()
{
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("background-color: #333333;border: none;");
    setAcceptDrops(true);
    showCut(false);
}

void CustomView::mousePressEvent(QMouseEvent* event)
{
    if (!m_showCut)
        return;
    if (event->button() == Qt::LeftButton)
    {
        m_startPoint = mapToScene(event->pos());
        // 如果之前有矩形，先移除
        if (m_rectItem)
        {
            scene()->removeItem(m_rectItem);
            delete m_rectItem;
            m_rectItem = nullptr;
        }
        // 创建一个新的 QGraphicsRectItem
        m_rectItem = new QGraphicsRectItem();
        m_rectItem->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        m_rectItem->setBrush(QBrush(QColor(0, 0, 255, 50)));  // 半透明蓝色
        scene()->addItem(m_rectItem);
    }
    QGraphicsView::mousePressEvent(event);
}

void CustomView::mouseMoveEvent(QMouseEvent* event)
{
    // 绘制十字光标（可选，可以在 paintEvent 中处理）
    // 或者用 QCursor 改变光标样式
    if (!m_showCut)
        return;
    setCursor(Qt::CrossCursor);

    if (m_rectItem)
    {
        QPointF endPoint = mapToScene(event->pos());
        QRectF rect(m_startPoint, endPoint);
        m_rectItem->setRect(rect.normalized());
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CustomView::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_showCut)
        return;
    if (event->button() == Qt::LeftButton)
    {
        // 绘制完成，可以在这里处理裁剪区域
        if (m_rectItem)
        {
            QRectF cropRect = m_rectItem->rect();
            qDebug() << "Cropping rect: " << cropRect;
            // 可以在这里发送信号或者调用其他函数来处理这个矩形
        }
    }
    QGraphicsView::mouseReleaseEvent(event);

    showCut(false);
}