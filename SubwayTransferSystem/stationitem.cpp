#include "stationitem.h"
#include <QDebug>
#include <QPen>
#include <QPainter>
#include <QCursor>
#include <QToolTip>
#include <QFontMetrics>
#include <QApplication>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneMouseEvent>

StationItem::StationItem(const QString& text, const QPointF& position, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_icon(QPixmap(":/resource/icon/wuhanmetro.png").scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation))
    , m_position(position)
    , m_width(0)
    , m_height(0)
    , m_textWidth(0)
    , m_textHeight(0)
    , m_text(text)
{
    init();
}

void StationItem::init()
{
#if 0
    // set icon
    QPixmap icon(":/resource/icon/wuhanmetro.png");
    const QPixmap& scaledIcon = icon.scaled(24,24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_icon = new QGraphicsPixmapItem(this);
    m_icon->setPixmap(scaledIcon);
//    m_icon->setOffset(-scaledIcon.width() / 2, -scaledIcon.height() / 2);
    m_icon->setPos(0, 0);

    // set label
    m_label = new QGraphicsTextItem(this);
    m_label->setPlainText(QStringLiteral("循礼门"));
    m_label->setFont(s_labelFont);
    m_label->setDefaultTextColor(s_textColor);
    m_label->setPos(0, scaledIcon.height() + 5);

    // set indicator
    m_selectedIndicator = new QGraphicsRectItem(this);
    m_selectedIndicator->setPen(QPen(s_lineColor, 2, Qt::DotLine));
    m_selectedIndicator->setVisible(false);
    m_selectedIndicator->setRect(0, 0, scaledIcon.width(), scaledIcon.height() + m_label->boundingRect().height() + 5);
#endif

    // set position
    setPos(m_position);

    // calculate dimensions based on pixmap and text
    QFontMetrics fontMatrics(QApplication::font());
    m_textWidth = fontMatrics.horizontalAdvance(m_text);
    m_textHeight = fontMatrics.height();
    m_width = qMax<qreal>(m_icon.width(), m_textWidth);
    m_height = m_icon.height() + m_textHeight;

    // set flag
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);
}

void StationItem::setGlobalStyle(const QColor &lineColor, const QFont &labelFont, const QColor &textColor)
{
    s_lineColor = lineColor;
    s_labelFont = labelFont;
    s_textColor = textColor;
}

void StationItem::hoverEnterEvent(QGraphicsSceneHoverEvent *hoverEvent)
{
    QToolTip::showText(hoverEvent->screenPos(), toolTip());
    QGraphicsItem::hoverEnterEvent(hoverEvent);
}

void StationItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *hoverEvent)
{
    QToolTip::hideText();
    QGraphicsItem::hoverLeaveEvent(hoverEvent);
}

void StationItem::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton) {
        setSelected(!isSelected());
        update();
    }

    QGraphicsItem::mousePressEvent(mouseEvent);
}

QRectF StationItem::boundingRect() const
{
    return QRectF(-m_width / 2, -m_height / 2, m_width, m_height);
}

void StationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPointF pixTopLeft(-m_icon.width() / 2, -m_icon.height() / 2);
    painter->drawPixmap(pixTopLeft, m_icon);

    QRectF textRect(-m_width / 2, -m_height / 2 + m_icon.height(), m_width, m_textHeight);
    painter->drawText(textRect, Qt::AlignCenter, m_text);

    if (option->state & QStyle::State_Selected) {
        QPen pen(Qt::DashLine);
        pen.setWidth(2);
        painter->setPen(pen);
        painter->drawRect(boundingRect());
    }
}
