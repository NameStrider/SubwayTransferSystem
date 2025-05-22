#include "subwaygraphview.h"
#include "QWheelEvent"

SubwayGraphView::SubwayGraphView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(nullptr)
{
    m_scene = new SubwayGraphScene(this);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setSceneRect(m_scene->itemsBoundingRect());
}

void SubwayGraphView::wheelEvent(QWheelEvent *event)
{
    // 缩放因子（1.1表示每次放大10%，0.9表示每次缩小10%）
    const double scaleFactor = 1.1;

    if (event->angleDelta().y() > 0) {
        // 滚轮向上滚动 - 放大
        scale(scaleFactor, scaleFactor);
    }
    else {
        // 滚轮向下滚动 - 缩小
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }

    event->accept();
}
