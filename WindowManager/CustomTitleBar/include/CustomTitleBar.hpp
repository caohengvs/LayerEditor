#pragma once
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
};