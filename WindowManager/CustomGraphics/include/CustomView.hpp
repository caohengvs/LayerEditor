#pragma once
/**
 * @file: CustomView.hpp
 * @brief: 自定义的QGraphicsView类，用于扩展QGraphicsView的功能
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
    void init();
};