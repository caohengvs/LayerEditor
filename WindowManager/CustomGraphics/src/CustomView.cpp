#include "CustomView.hpp"
#include <QOpenGLWidget>

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

void CustomView::init()
{
    setViewport(new QOpenGLWidget());
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("background-color: #333333;border: none;");
}
