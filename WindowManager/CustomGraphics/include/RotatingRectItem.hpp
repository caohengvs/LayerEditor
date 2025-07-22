
#pragma once
#include <QGraphicsRectItem>
#include <QObject>
#include <QPropertyAnimation>
#include <future>

class RotatingRectItem
    : public QObject
    , public QGraphicsRectItem
{
    Q_OBJECT

    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)

private:
    using AnimationCallback = std::function<void()>;

public:
    explicit RotatingRectItem(const QRectF& rect, QGraphicsItem* parent = nullptr);
    ~RotatingRectItem() override;

    bool startRotationAnimation(AnimationCallback callback, int durationMs = 2000);

signals:
    void finished();

private:
    QPropertyAnimation* m_animation;
    std::future<void> m_future;
};
