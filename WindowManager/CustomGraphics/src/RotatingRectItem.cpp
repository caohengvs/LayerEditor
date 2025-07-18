#include "RotatingRectItem.hpp"
#include <QBrush>
#include <QDebug>
#include <QPen>

RotatingRectItem::RotatingRectItem(const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent)
{
    setTransformOriginPoint(rect.center());

    m_animation = new QPropertyAnimation(this, "rotation", this);
    m_animation->setLoopCount(-1);
    setPen(QPen(Qt::blue, 2));
    setBrush(QBrush(Qt::transparent));
}

RotatingRectItem::~RotatingRectItem()
{
    qDebug() << "RotatingRectItem destructor called";
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