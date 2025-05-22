#ifndef STATIONITEM_H
#define STATIONITEM_H

#include <QFont>
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsItemGroup>

class StationItem : public QGraphicsItem
{
public:
    StationItem(const QString& text, const QPointF& position = QPoint(0, 0), QGraphicsItem* parent = nullptr);

    void init();

    QPointF position() const { return m_position; }

    void setPosition(const QPointF& position) { m_position = position; }

    static void setGlobalStyle(const QColor& lineColor
                               , const QFont& labelFont = QFont("Microsoft YaHei", 8)
                               , const QColor& textColor = Qt::black);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* hoverEvent) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* hoverEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
    // define the bounding rectangle, centered at (0,0)
    QRectF boundingRect() const override;
    void paint(QPainter* painter
               , const QStyleOptionGraphicsItem* option
               , QWidget* widget = nullptr) override;

private:
    QPixmap m_icon;
    QPointF m_position;
    qreal m_width;
    qreal m_height;
    qreal m_textWidth;
    qreal m_textHeight;
    QString m_text;

    // global style
    inline static QColor s_lineColor = QColor(255, 105, 0);
    inline static QFont s_labelFont = QFont("Microsoft YaHei", 8);
    inline static QColor s_textColor = Qt::black;
};

#endif // STATIONITEM_H
