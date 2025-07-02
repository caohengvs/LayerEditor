
/**
 * @file CustomView.hpp
 * @brief Custom graphics view class for the WindowManager project.
 * @author Curtis
 * @email caohengvs888@gmail.com
 * @date 2025-07-01 16:19:20
 * @version 1.0
 */

#pragma once
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