#include "mainwindow.h"
#include "subwaygraph.h"
#include "jsonparser.h"
#include "jsongenerator.h"
#include "networkmanager.h"
#include "loguru.hpp"
#include <QApplication>
#include <QObject>

#define NETWORK_TEST

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    loguru::init(argc, argv);
    loguru::add_file("D:\\Qt_Projects\\subway_transfer_system.log", loguru::Append, loguru::Verbosity_MAX);

#ifdef SUBWAY_GRAPH_TEST
    SubwayGraph::StationNodeParams params;
    StationNodeParam paramA{"A", 120.0000, 30.0000, 3, QSet<int>{1, 4}};
    StationNodeParam paramB{"B", 121.0000, 31.0000, 3, QSet<int>{1, 2}};
    StationNodeParam paramC{"C", 122.0000, 32.0000, 2, QSet<int>{1}};
    StationNodeParam paramD{"D", 123.0000, 33.0000, 3, QSet<int>{1, 4}};
    StationNodeParam paramE{"E", 124.0000, 34.0000, 2, QSet<int>{2}};
    StationNodeParam paramF{"F", 125.0000, 35.0000, 3, QSet<int>{4}};
    StationNodeParam paramG{"G", 126.0000, 36.0000, 3, QSet<int>{2, 4}};
    StationNodeParam paramH{"H", 127.0000, 37.0000, 3, QSet<int>{4}};
    params.push_back(paramA);
    params.push_back(paramB);
    params.push_back(paramC);
    params.push_back(paramD);
    params.push_back(paramE);
    params.push_back(paramF);
    params.push_back(paramG);
    params.push_back(paramH);

    SubwayGraph::LineNames lineNames;
    lineNames[1] = QVector<QString>{"A", "B", "C", "D"};
    lineNames[2] = QVector<QString>{"E", "B", "G"};
    lineNames[4] = QVector<QString>{"D", "F", "H", "G", "A"};

    SubwayGraph::LineDistances lineDistances;
    lineDistances[1] = QVector<int>{2, 3, 2};
    lineDistances[2] = QVector<int>{3, 2};
    lineDistances[4] = QVector<int>{2, 2, 2, 3};

    SubwayGraph subwayGraph;
    QString err_msg;
    if (subwayGraph.build(params, lineNames, lineDistances, err_msg) != true) {
        qDebug() << err_msg;
    }
    else {
       printSubwayGraph(subwayGraph);
    }

    StationNodeParam param("I", 128.0000, 38.0000, 3, QSet<int>{2, 4});
    SubwayGraph::MultiBiDirectionStations biStations;
    biStations[2] = BiDirectionStations(StationDistance(), StationDistance("E", 4));
    biStations[4] = BiDirectionStations(StationDistance("A", 3), StationDistance());
    if (subwayGraph.addNode(param, biStations, err_msg)) {
        printSubwayGraph(subwayGraph);
    }
    else {
        qDebug() << err_msg;
    }

    if (subwayGraph.removeNode("I", biStations, err_msg)) {
        printSubwayGraph(subwayGraph);
    }
    else {
        qDebug() << err_msg;
    }

    subwayGraph.clear();
    printSubwayGraph(subwayGraph);

    lineNames.clear();
    lineDistances.clear();
    params.clear();
    JsonParser::getJsonParserInstance().setSubwayGraph(&subwayGraph);
    if (JsonParser::getJsonParserInstance().parse(err_msg) != true) {
        qDebug() << err_msg;
    }
    else {
        lineNames = JsonParser::getJsonParserInstance().m_lineNames;
        lineDistances = JsonParser::getJsonParserInstance().m_lineDistances;
        params = JsonParser::getJsonParserInstance().m_stationNodeParams;
        if (subwayGraph.build(params, lineNames, lineDistances, err_msg) != true) {
            qDebug() << err_msg;

            QStringList context;
            for (const auto& it : lineNames) {
                for (const QString& name : it) {
                    context << name;
                }
                qDebug() << context.join(" ");
                context.clear();
            }

            for (const auto& it : lineDistances) {
                for (const int& distance : it) {
                    context << QString::number(distance);
                }
                qDebug() << context.join(" ");
                context.clear();
            }

            for (const auto& param : params) {
                qDebug() << param.name << param.longitude << param.latitude << param.stayTime;
            }
        }
        else {
           printSubwayGraph(subwayGraph);
        }
    }
#endif

#ifdef JSON_TEST
    JsonParser& parser = JsonParser::getJsonParserInstance();
    SubwayGraph graph;
    QObject::connect(&parser, &JsonParser::parseFinished, &graph, &SubwayGraph::startBuild);
    parser.parse();
    printSubwayGraph(graph);
    SubwayGraph::PathInfo pathInfo = graph.bfs("A", "F");
    const SubwayGraph::PathInfo::Path& path = pathInfo.path;
    qDebug() << pathInfo.totalDistance;
    QStringList stationList;
    for (const QString& station : path) {
        stationList << station;
    }
    qDebug() << stationList.join(" ");

#endif

#ifdef NETWORK_TEST
    NetworkManager networkManager;
    QUrl url(DEFAULT_WUHAN_METRO_REQUEST_URL);
    networkManager.request(url);
    JsonGenerator& jsonGenerator = JsonGenerator::getJsonGenerator();
    JsonParser& jsonParser = JsonParser::getJsonParserInstance();
    SubwayGraph subwayGraph;
    QObject::connect(&networkManager, &NetworkManager::requestFinished, &jsonGenerator, &JsonGenerator::generate);
    QObject::connect(&jsonGenerator, &JsonGenerator::generateFinished, &jsonParser, &JsonParser::parse);
    QObject::connect(&jsonParser, &JsonParser::parseFinished, &subwayGraph, &SubwayGraph::startBuild);
#endif

    return a.exec();
}
