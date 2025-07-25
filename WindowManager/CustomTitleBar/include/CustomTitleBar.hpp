#pragma once
/**
 * @file: CustomTitleBar.hpp
 * @brief: 自定义标题栏类，用于替代默认的窗口标题栏
 * @author: Curtis
 * @date: 2025-07-16 14:14:20
 * @version: 1.0
 * @email: caohengvs888@gmail.com
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPushButton>
#include <QWidget>

class CustomTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit CustomTitleBar(QMainWindow* mainWindow, QWidget* parent = nullptr);
    ~CustomTitleBar();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
    void onCloseClicked();

private:
    QMainWindow* m_parentWindow;
    QPushButton* m_closeButton;

    QPoint m_dragPosition;
    bool m_isDragging = false;
    QTimer* m_timer;  
};