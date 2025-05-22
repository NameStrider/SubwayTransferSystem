#ifndef LINEITEM_H
#define LINEITEM_H

#include <QGraphicsPathItem>

class LineItem : public QGraphicsPathItem
{
public:
    LineItem(const QPointF& from, const QPointF& to, QGraphicsItem* parent = nullptr);

protected:
    void paint(QPainter* painter,
                  const QStyleOptionGraphicsItem* option,
                  QWidget* widget = nullptr) override;

private:
    QPointF m_from;
    QPointF m_to;
};

#endif // LINEITEM_H
