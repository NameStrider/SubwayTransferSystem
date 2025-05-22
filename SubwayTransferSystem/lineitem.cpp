#include "lineitem.h"
#include <QPainter>

LineItem::LineItem(const QPointF& from, const QPointF& to, QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
    , m_from(from)
    , m_to(to)
{
    QPainterPath path;
    path.moveTo(m_from);
    path.lineTo(m_to);
    setPath(path);
}

void LineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.moveTo(m_from);
    path.lineTo(m_to);
    QPen pen;
    pen.setWidth(2);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawPath(path);

    QGraphicsPathItem::paint(painter, option, widget);
}
