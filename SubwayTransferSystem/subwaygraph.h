#ifndef SUBWAYGRAPH_H
#define SUBWAYGRAPH_H

#include "stationnode.h"
#include "linelist.h"
#include <QHash>
#include <QList>
#include <QSharedPointer>

class SubwayGraph;

void printSubwayGraph(const SubwayGraph& subwayGraph);

struct Edge
{
    QSharedPointer<StationNode> toNode;
    int distance;

    Edge(const QSharedPointer<StationNode>& node, int dist)
        : toNode(node)
        , distance(dist)
    {}
};

struct StationDistance {
    QString name;
    int distance;

    StationDistance()
        : name("")
        , distance(0)
    {}

    StationDistance(const QString& _name, int _distance)
        : name(_name)
        , distance(_distance)
    {}

    bool isValid() const;
};

struct BiDirectionStations {
    StationDistance preStation;
    StationDistance nextStation;

    BiDirectionStations()
        : preStation()
        , nextStation()
    {}

    BiDirectionStations(const StationDistance& pre, const StationDistance& next)
        : preStation(pre)
        , nextStation(next)
    {}

    bool isValid(const SubwayGraph& graph) const;
};


class SubwayGraph : public QObject
{
public:
    // 邻接表
    using Graph = QHash<QString, QList<Edge>>;

    // 换乘路径，(int: total distance, QString: station name)
    // 不能是 QPair<int, QSharedPointer<StationNode>> 因为 StationNode 需要被析构
    using Path = QPair<int, QList<QString>>;

    // 站点集合
    using Stations = QHash<QString, QSharedPointer<StationNode>>;

    // 线路集合
    using Lines = QVector<LineList>;

    using StationNodeParams = QVector<StationNodeParam>;

    using LineNames = QHash<int, QVector<QString>>;

    using LineDistances = QHash<int, QVector<int>>;

    using MultiBiDirectionStations = QHash<int, BiDirectionStations>;

    SubwayGraph(QObject* parent = nullptr);

    // 找不到返回空智能指针，安全性？
    QSharedPointer<StationNode> getNode(const QString& name) const;

    bool addNode(const StationNodeParam& param, const MultiBiDirectionStations& biStations, QString& err_msg);

    bool removeNode(const QString& name, const MultiBiDirectionStations& biStations, QString& err_msg);

    // 先 buildStations 然后 buildLines 最后 buildGraph
    bool build(const StationNodeParams& stationParams, const LineNames& lines
               , const LineDistances& distances, QString& err_msg);

    Path dfs() const;

    Path bfs() const;

    const Stations& getStations() const { return m_stations; }
    const Graph& getGraph() const { return m_graph; }
    const Lines& getLines() const { return m_lines; }

private:
    bool buildStations(const StationNodeParams& stationParams,QString& err_msg);

    bool buildLines(const LineNames& lines, QString& err_msg);

    bool buildGraph(const LineDistances& distances, QString& err_msg);

    bool addNodeToSet(const StationNodeParam& param);

    bool removeNodeFromSet(const QString& name);

    bool addNodeToLine(const QString& name, int lineId, const QString& preName);

    bool removeNodeFromLine(const QString& name, int lineId);

    bool addNodeToGraph(const QSharedPointer<StationNode>& node, const MultiBiDirectionStations& biStations);

    bool removeNodeFromGraph(const QSharedPointer<StationNode>& node, const MultiBiDirectionStations& biStations);

    bool addEdgeToGraph(const QString& name, const Edge& edge);

    bool removeEdgeFromGraph(const QString& name, const QSharedPointer<StationNode>& toNode);

    bool removeEdgesFromGraph(const QString& name);

    bool m_isBuildLineSucess;
    bool m_isBuildGraphSucess;
    Graph m_graph;
    Stations m_stations;
    Lines m_lines;
};

#endif // SUBWAYGRAPH_H
