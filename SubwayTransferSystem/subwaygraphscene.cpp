#include "subwaygraphscene.h"

const QPointF g_referencePoint(114.013537, 30.770759);

QPointF SubwayGraphScene::coorperateTransform(qreal longitude, qreal latitude) {
    QPointF point;
    point.rx() = abs(longitude - g_referencePoint.x()) * m_scaleFactor;
    point.ry() = abs(latitude - g_referencePoint.y()) * m_scaleFactor;
    return point;
}

SubwayGraphScene::SubwayGraphScene(QObject* parent)
    : QGraphicsScene(parent)
    , m_scaleFactor(200.00)
{
    StationItem::setGlobalStyle(QColor(0, 160, 230), QFont("微软雅黑", 9, QFont::Bold), QColor(80, 80, 80));

    QPointF from = coorperateTransform(114.50, 30.10);
    QPointF to = coorperateTransform(114.60, 30.20);
    StationItem* stationItemA = new StationItem("循礼门", from);
    StationItem* stationItemB = new StationItem("积玉桥", to);
    LineItem* lineItemAB = new LineItem(from, to);

    addItem(stationItemA);
    addItem(stationItemB);
    addItem(lineItemAB);


    QUrl url(DEFAULT_WUHAN_METRO_REQUEST_URL);
    m_networkManager.request(url);
    JsonGenerator& jsonGenerator = JsonGenerator::getJsonGenerator();
    JsonParser& jsonParser = JsonParser::getJsonParserInstance();
    QObject::connect(&m_networkManager, &NetworkManager::requestFinished, &jsonGenerator, &JsonGenerator::generate);
    QObject::connect(&jsonGenerator, &JsonGenerator::generateFinished, &jsonParser, &JsonParser::parse);
    QObject::connect(&jsonParser, &JsonParser::parseFinished, &m_subwayGraph, &SubwayGraph::startBuild);
    QObject::connect(&m_subwayGraph, &SubwayGraph::buildFinished, this, &SubwayGraphScene::paintSubwayGraph);

}

void SubwayGraphScene::paintSubwayGraph(const SubwayGraph::Stations &stations, const SubwayGraph::Lines &lines)
{
    for (auto station = stations.begin(); station != stations.end(); ++station) {
        qreal latitude = station.value()->param.latitude;
        qreal longitude = station.value()->param.longitude;
        QPointF point = coorperateTransform(longitude, latitude);
        StationItem* item = new StationItem(station.key(), point);

        addItem(item);
    }

    for (auto it = lines.begin(); it != lines.end(); ++it) {
        if (it->size() <= 0) {
            continue;
        }
        const QList<QSharedPointer<StationNode>>& line = it->getLineList();
        for (auto node = line.begin(); node != line.end() - 1; ++node) {
            qreal latitudeFrom = (*node)->param.latitude;
            qreal longitudeFrom = (*node)->param.longitude;
            qreal latitudeTo = (*(node + 1))->param.latitude;
            qreal longitudeTo = (*(node + 1))->param.longitude;

            QPointF from = coorperateTransform(longitudeFrom, latitudeFrom);
            QPointF to = coorperateTransform(longitudeTo, latitudeTo);

            LineItem* item = new LineItem(from, to);

            addItem(item);
        }
    }
}
