#include "RotatingRectItem.hpp"
#include <QBrush>
#include <QConicalGradient>
#include <QDebug>
#include <QPainter>
#include <QPen>


RotatingRectItem::RotatingRectItem(const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent)
{
    setTransformOriginPoint(rect.center());
    m_animation = new QPropertyAnimation(this, "rotation", this);
    m_animation->setLoopCount(-1);
    m_animation->setEasingCurve(QEasingCurve::Linear);
    setPen(Qt::NoPen);
    setBrush(Qt::NoBrush);
}

RotatingRectItem::~RotatingRectItem()
{
    qDebug() << "RotatingRectItem destructor called";
}

void RotatingRectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    QRectF r = rect().adjusted(8, 8, -8, -8);  // 留出宽度
    qreal arcWidth = 12.0;

    // 渐变色
    QConicalGradient gradient(r.center(), -rotation());
    gradient.setColorAt(0.0, QColor(0, 120, 255, 255));  // 深色
    gradient.setColorAt(0.7, QColor(0, 200, 255, 180));  // 浅色
    gradient.setColorAt(1.0, QColor(0, 120, 255, 0));    // 透明

    QPen pen(QBrush(gradient), arcWidth, Qt::SolidLine, Qt::RoundCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // 画一段弧（比如270°）
    int spanAngle = 270 * 16;  // Qt角度单位为1/16°
    int startAngle = 0 * 16;
    painter->drawArc(r, startAngle, spanAngle);
}

bool RotatingRectItem::startRotationAnimation(AnimationCallback callback, int durationMs)
{
    if (m_animation->state() == QPropertyAnimation::Running)
    {
        qDebug() << "Animation is already running.";
        return false;
    }

    m_animation->setDuration(durationMs);
    m_animation->setStartValue(0);
    m_animation->setEndValue(360);
    m_animation->start();

    m_future = std::async(std::launch::async,
                          [this, callback]()
                          {
                              if (callback)
                              {
                                  callback();
                              }
                              emit finished();
                          });

    return true;
}