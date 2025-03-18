#include "subwaygraph.h"
#include <QDebug>

SubwayGraph::SubwayGraph(QObject* parent)
    : QObject(parent)
    , m_isBuildLineSucess(false)
    , m_isBuildGraphSucess(false)
{

}

QSharedPointer<StationNode> SubwayGraph::getNode(const QString &name) const
{
    auto node = m_stations.find(name);
    if (node == m_stations.end())
        return QSharedPointer<StationNode>();
    return *node;
}

bool SubwayGraph::addNode(const StationNodeParam& param, const MultiBiDirectionStations& biStations, QString& err_msg)
{
    if (m_stations.size() > MAX_STATION_NODE) {
        err_msg = QString("add node %1 failed as node count reached maximum limit").arg(param.name);
        return false;
    }
    if (!param.isValid()) {
        err_msg = QString("add node %1 failed as param invalid").arg(param.name);
        return false;
    }
    if (param.belongingLines.size() != biStations.size()) {
        err_msg = QString("the size of belonging lines of node %1 mismatch with bi-stations").arg(param.name);
        return false;
    }
    for (auto line = param.belongingLines.begin(); line != param.belongingLines.end(); ++line) {
        int lineId = *line;
        if (biStations.find(lineId) == biStations.end()) {
            err_msg = QString("add node %1 failed as param mismatch with bi-stations").arg(param.name);
            return false;
        }
        else {
            if (!biStations[lineId].isValid(*this)) {
                err_msg = QString("add node %1 failed as bi-stations invalid").arg(param.name);
                return false;
            }
        }
    }

    // 先添加到 m_stations 中，然后添加到 m_lines ，最后添加到 m_graph
    if (!addNodeToSet(param)) {
        err_msg = QString("add node %1 to set failed").arg(param.name);
        return false;
    }

    for (auto it = biStations.begin(); it != biStations.end(); ++it) {
       if (!addNodeToLine(param.name, it.key(), it.value().preStation.name)) {
           err_msg = QString("add node %1 to line %2 failed").arg(param.name).arg(it.key());
           return false;
       }
    }

    if (!addNodeToGraph(getNode(param.name), biStations)) {
        err_msg = QString("add node %1 to graph failed").arg(param.name);
        return false;
    }

    return true;
}

bool SubwayGraph::removeNode(const QString &name, const MultiBiDirectionStations& biStations, QString& err_msg)
{
    if (m_stations.find(name) == m_stations.end()) {
        err_msg = QString("remove node %1 failed as it is not existed").arg(name);
        return false;
    }

    // 先从 Lines 中移除，然后从 Graph 中移除，最后从 Stations 中移除
    const QSharedPointer<StationNode>& node = m_stations[name];
    const QSet<int>& belongsLines = node->param.belongingLines;
    for (auto biStation = biStations.begin(); biStation != biStations.end(); ++biStation) {
        int lineId = biStation.key();
        if (belongsLines.find(lineId) == belongsLines.end()) {
            err_msg = QString("remove node %1 failed as it does not belong to line %2").arg(name).arg(lineId);
            return false;
        }
        if (!removeNodeFromLine(name, lineId)) {
            err_msg = QString("remove node %1 failed when removing it from line %2").arg(name).arg(lineId);
            return false;
        }
    }
    if (!removeNodeFromGraph(node, biStations)) {
        err_msg = QString("remove node %1 failed when removing it from graph").arg(name);
        return false;
    }

    // 最后判断是否需要从 Stations 中移除
    if (belongsLines.size() < 1)
        m_stations.remove(name);

    return true;
}

bool SubwayGraph::build(const SubwayGraph::StationNodeParams &stationParams, const SubwayGraph::LineNames &lineNames
                        , const SubwayGraph::LineDistances &lineDistances, QString &err_msg)
{
    if (buildStations(stationParams, err_msg) != true
        || buildLines(lineNames, err_msg) != true
        || buildGraph(lineDistances, err_msg) != true)
        return false;
    return true;
}

void SubwayGraph::clear()
{
    m_graph.clear();
    m_stations.clear();
    m_lines.clear();
}

SubwayGraph::Path SubwayGraph::dfs() const
{
    return Path();
}

SubwayGraph::Path SubwayGraph::bfs() const
{
    return Path();
}

void SubwayGraph::startBuild(const SubwayGraph::LineNames &lineNames, const SubwayGraph::LineDistances &lineDistances, const SubwayGraph::StationNodeParams &nodeParams)
{
    QString err_msg;
    if (build(nodeParams, lineNames, lineDistances, err_msg) != false) {
        qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
        printSubwayGraph(*this);
    }
    else {
        qDebug() << err_msg;
    }
}

bool SubwayGraph::buildStations(const StationNodeParams& stationParams, QString& err_msg)
{
    for (auto nodeParam : stationParams) {
        if (!addNodeToSet(nodeParam)) {
            err_msg = QString("build stations failed as add node to set failed");
            return false;
        }
    }
    return true;
}

bool SubwayGraph::buildLines(const SubwayGraph::LineNames& lines, QString& err_msg)
{
    for (auto line = lines.begin(); line != lines.end(); ++line) {
        // 一条线路至少要有 MIN_STATION_IN_LINE 个站点
        if (line.value().size() < MIN_STATION_IN_LINE) {
            err_msg = QString("build line %1 failed as too small station in line").arg(line.key());
            return false;
        }

        int lineId = line.key();
        QString preName;
        for (const auto& station : line.value()) {
            if (!addNodeToLine(station, lineId, preName))
                return false;
            preName = station;
        }
    }

    m_isBuildLineSucess = true;
    return true;
}

// 前提是 buildLines 成功
bool SubwayGraph::buildGraph(const SubwayGraph::LineDistances& distances, QString& err_msg)
{
    if (m_isBuildLineSucess != true) {
        err_msg = QString("build graph failed as build line failed");
        return false;
    }

    for (const auto& line : m_lines) {
        // 该线路未规划，线路id不一定连续
        if (!line.isValid())
            continue;

        int lineId = line.lineId();
        LineDistances::ConstIterator dist = distances.find(lineId);
        if (dist == distances.end()) {
            err_msg = QString("build graph failed as line %1 is not exited").arg(lineId);
            return false;
        }

        // 站点数量比距离数量多1
        if (dist.value().size() != line.size() - 1) {
            err_msg = QString("build graph failed as station count of line %1 mismatch with distance count").arg(lineId);
            return false;
        }

        int i = 0;
        const QList<QSharedPointer<StationNode>>& lineList = line.getLineList();
        // 迭代器每次向后移动两次，否则每个 edge 都会被添加两次
        for (auto station = lineList.begin(); station < lineList.end(); ++station, ++station) {
            MultiBiDirectionStations multiStations;
            const auto& node = *station;
            if (station == lineList.begin()) {
                auto next = station + 1;
                QString nextName = (*(next))->param.name;
                int nextDistance = (*dist)[i];
                BiDirectionStations biStations(StationDistance("", 0), StationDistance(nextName, nextDistance));
                multiStations[lineId] = biStations;
            }
            else if (station == lineList.end() - 1) {
                auto pre = station - 1;
                QString preName = (*pre)->param.name;
                int preDistance = (*dist)[i - 1];
                BiDirectionStations biStations(StationDistance(preName, preDistance), StationDistance("", 0));
                multiStations[lineId] = biStations;
            }
            else {
                auto pre = station - 1;
                auto next = station + 1;
                QString preName = (*pre)->param.name;
                QString nextName = (*(next))->param.name;
                int preDistance = (*dist)[i - 1];
                int nextDistance = (*dist)[i];
                BiDirectionStations biStations(StationDistance(preName, preDistance), StationDistance(nextName, nextDistance));
                multiStations[lineId] = biStations;
            }
            i += 2; // 移动两次
            if (!addNodeToGraph(node, multiStations)) {
                err_msg = QString("build graph failed as add node to graph failed");
                return false;
            }
        }
    }

    m_isBuildGraphSucess = true;
    return true;
}

bool SubwayGraph::addNodeToSet(const StationNodeParam& param)
{
    if (m_stations.size() > MAX_STATION_NODE)
        return false;

    // 参数判断
    if (!param.isValid())
        return false;

    if (m_stations.find(param.name) != m_stations.end())
        return true;

    m_stations[param.name] = QSharedPointer<StationNode>::create(param);
    return true;
}

// 先从 Lines 和 Graph 中 remove，最后从 Stations 中 remove
bool SubwayGraph::removeNodeFromSet(const QString &name)
{
    if (m_stations.find(name) == m_stations.end())
        return false;

    m_stations.remove(name);
    return true;
}

bool SubwayGraph::addNodeToLine(const QString &name, int lineId, const QString &preName)
{
    // 前提是 name 已经添加到 m_stations
    if (m_stations.find(name) == m_stations.end())
        return false;

    // lineId 是否有效
    if (m_lines.size() - 1 < lineId)
        m_lines.resize(lineId + 1);

    m_lines[lineId].setLineId(lineId);

    if (preName.isEmpty()) {
        m_lines[lineId].addNode(m_stations[name]);
    }
    else {
        if (m_stations.find(preName) == m_stations.end())
            return false;
        else if (m_stations[preName]->param.belongingLines.find(lineId)
                 == m_stations[preName]->param.belongingLines.end()) {
            return false;
        }
        else {
            m_lines[lineId].addNode(m_stations[name], preName);
        }
    }

    return true;
}

bool SubwayGraph::removeNodeFromLine(const QString &name, int lineId)
{
    if (m_stations.find(name) == m_stations.end())
        return false;

    if (lineId >= m_lines.size())
        return false;

    if (!m_lines[lineId].isValid())
        return false;

    // 修改 name 对应的 StationNode::Param
    m_stations[name]->param.belongingLines.remove(lineId);
    m_lines[lineId].removeNode(name);
    return true;
}

bool SubwayGraph::addNodeToGraph(const QSharedPointer<StationNode>& node, const MultiBiDirectionStations& biStations)
{
    if (getNode(node->param.name) == nullptr)
        return false;

    for (auto biStation = biStations.begin(); biStation != biStations.end(); ++biStation) {
        if (biStation.value().isValid(*this) == false)
            return false;

        auto pre = getNode(biStation.value().preStation.name);
        auto next = getNode(biStation.value().nextStation.name);
        if (pre && next) {
            removeEdgeFromGraph(pre->param.name, next);
            addEdgeToGraph(pre->param.name, Edge(node, biStation.value().preStation.distance));

            addEdgeToGraph(node->param.name, Edge(pre, biStation.value().preStation.distance));
            addEdgeToGraph(node->param.name, Edge(next, biStation.value().nextStation.distance));

            removeEdgeFromGraph(next->param.name, pre);
            addEdgeToGraph(next->param.name, Edge(node, biStation.value().nextStation.distance));
        }
        else if (!pre && next) {
            addEdgeToGraph(node->param.name, Edge(next, biStation.value().nextStation.distance));
            addEdgeToGraph(next->param.name, Edge(node, biStation.value().nextStation.distance));
        }
        else if (pre && !next) {
            addEdgeToGraph(node->param.name, Edge(pre, biStation.value().preStation.distance));
            addEdgeToGraph(pre->param.name, Edge(node, biStation.value().preStation.distance));

        }
        else {

        }
    }

    return true;
}

bool SubwayGraph::removeNodeFromGraph(const QSharedPointer<StationNode> &node, const MultiBiDirectionStations &biStations)
{
    for (const auto& stations : biStations) {
        const QString& preName = stations.preStation.name;
        const QString& nextName = stations.nextStation.name;
        auto pre = getNode(preName);
        auto next = getNode(nextName);
        int distanceFromPreToNext = stations.preStation.distance + stations.nextStation.distance;

        if (pre && next) {
            removeEdgeFromGraph(preName, node);
            addEdgeToGraph(preName, Edge(next, distanceFromPreToNext));

            removeEdgeFromGraph(nextName, node);
            addEdgeToGraph(nextName, Edge(pre, distanceFromPreToNext));
        }
        else if (!pre && next) {
            removeEdgeFromGraph(nextName, node);
        }
        else if (pre && !next) {
            removeEdgeFromGraph(preName, node);
        }
        else {

        }

        removeEdgesFromGraph(node->param.name);
    }

    return true;
}

// 添加的是 edge
bool SubwayGraph::addEdgeToGraph(const QString& name, const Edge& edge)
{   
    // 前提是 name 和 edge.toNode 已经添加到站点集合了
    if (m_stations.find(name) == m_stations.end()
        || edge.toNode == nullptr
        || m_stations.find(edge.toNode->param.name) == m_stations.end())
        return false;

    if (m_graph.find(name) == m_graph.end()) {
        m_graph.insert(name, QList<Edge>());
    }

    m_graph[name].push_back(edge);
    return true;
}

// 移除 edge
bool SubwayGraph::removeEdgeFromGraph(const QString &name, const QSharedPointer<StationNode>& toNode)
{
    if (m_stations.find(name) == m_stations.end())
        return false;
    else if (m_graph.find(name) == m_graph.end())
        return false;

    auto pos = m_graph[name].begin();
    for (; pos != m_graph[name].end(); ++pos) {
        if ((*pos).toNode == toNode)
            break;
    }
    if (pos != m_graph[name].end()) {
        m_graph[name].erase(pos);
        // 链表空，移除这一项
        if (m_graph[name].size() == 0)
            removeEdgesFromGraph(name);
        return true;
    }
    return false;
}

// 移除 name 对应的整个链表
bool SubwayGraph::removeEdgesFromGraph(const QString &name)
{
    if (m_graph.find(name) != m_graph.end()) {
        m_graph.remove(name);
    }
    return true;
}

bool StationDistance::isValid() const
{
    if (name.isEmpty()) {
        if (distance > 0)
           return false;
    }
    else {
        if (distance < MIN_DISTANCE || distance > MAX_DISTANCE)
            return false;
    }

    return true;
}

bool BiDirectionStations::isValid(const SubwayGraph &graph) const
{
    if (!preStation.isValid() || !nextStation.isValid())
        return false;
    if (!preStation.name.isEmpty()) {
        if (graph.getNode(preStation.name) == nullptr)
            return false;
    }
    if (!nextStation.name.isEmpty()) {
        if (graph.getNode(nextStation.name) == nullptr)
            return false;
    }
    return true;
}

void printSubwayGraph(const SubwayGraph& subwayGraph)
{
    // print m_stations
    const SubwayGraph::Stations& stations = subwayGraph.getStations();
    for (auto it_stations = stations.begin(); it_stations != stations.end(); ++it_stations) {
        qDebug() << it_stations.key() << it_stations.value();
    }

    // print m_lines
    const SubwayGraph::Lines& lines = subwayGraph.getLines();
    for (const auto& it_lines : lines) {
        if (!it_lines.isValid())
            continue;

        QStringList context;
        context << QString::number(it_lines.lineId());
        for (auto station : it_lines.getLineList()) {
            context << station->param.name;
        }
        qDebug() << context.join(" ");
    }

    // print m_graph
    const SubwayGraph::Graph& graph = subwayGraph.getGraph();
    for (auto it_graph = graph.begin(); it_graph != graph.end(); ++it_graph) {
        QStringList context;
        context << it_graph.key();
        for (auto edge = it_graph.value().begin(); edge != it_graph.value().end(); ++edge) {
            context << QString("(%1, %2)").arg((*edge).toNode->param.name).arg((*edge).distance);
        }
        qDebug() << context.join(" -> ");
    }
}
