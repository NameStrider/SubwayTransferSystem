#ifndef SUBWAYGRAPHSCENE_H
#define SUBWAYGRAPHSCENE_H

#include "lineitem.h"
#include "stationitem.h"
#include "subwaygraph.h"

#include "jsonparser.h"
#include "jsongenerator.h"
#include "networkmanager.h"

#include <QGraphicsScene>

class SubwayGraphScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit SubwayGraphScene(QObject* parent = nullptr);

    QPointF coorperateTransform(qreal longitude, qreal latitude);

public slots:
    void paintSubwayGraph(const SubwayGraph::Stations& stations, const SubwayGraph::Lines& lines);

private:
    qreal m_scaleFactor;

    SubwayGraph m_subwayGraph;
    NetworkManager m_networkManager;
};

#endif // SUBWAYGRAPHSCENE_H
