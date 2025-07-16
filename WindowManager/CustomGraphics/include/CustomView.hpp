#pragma once
/**
 * @file: CustomView.hpp
 * @brief: QGraphicsView, a custom view for displaying graphics scenes
 * @author: Curtis
 * @date: 2025-07-16 14:13:32
 * @version: 1.0
 * @email: caohengvs888@gmail.com
 */

#include <QGraphicsView>
#include <QLabel>
class CustomView final : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CustomView(QWidget* parent = nullptr);
    explicit CustomView(QGraphicsScene* scene, QWidget* parent = nullptr);
    ~CustomView() override;

private:
    void wheelEvent(QWheelEvent* event) override;

private:
    void init();
};