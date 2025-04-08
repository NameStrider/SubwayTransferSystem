# bug

# buildLines 失败，程序异常退出

```c++
bool SubwayGraph::addNodeToLine(const QString &name, int lineId, const QString &preName)
{
    // 前提是 name 已经添加到 m_stations
    if (m_stations.find(name) == m_stations.end())
        return false;

    // lineId 是否有效
    if (m_lines.size() - 1 < lineId) {
        m_lines.resize(lineId + 1);
        m_lines[lineId].setLineId(lineId);
        
    }
    else {
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
    }

    return true;
}
```



# buildGraph failed

- build graph failed as station count of line 1 mismatch with distance count

- ```
  "add node E to line 2"
  "add node B to line 2"
  "add node G to line 2"
  "add node A to line 1"
  "add node B to line 1"
  "add node C to line 1"
  "add node D to line 1"
  "add node D to line 4"
  "add node F to line 4"
  "add node H to line 4"
  "add node G to line 4"
  "add node A to line 4"
  "1 A B C D B C D"
  "2 E B G B G"
  "4 D F H G A F H G A"
  ```

  

  ```c++
  bool SubwayGraph::buildLines(const SubwayGraph::LineNames& lines, QString& err_msg)
  {
      for (auto line = lines.begin(); line != lines.end(); ++line) {
          // 一条线路至少要有 MIN_STATION_IN_LINE 个站点
          if (line.value().size() < MIN_STATION_IN_LINE) {
              err_msg = QString("build line %1 failed as too small station in line").arg(line.key());
              return false;
          }
  
          printSubwayGraph(*this);
          int lineId = line.key();
          QString preName;
          for (const auto& station : line.value()) {
              if (!addNodeToLine(station, lineId, preName))
                  return false;
              preName = station;
          }
      }
  
      printSubwayGraph(*this);
  
      m_isBuildLineSucess = true;
      return true;
  }
  
  ```

  

  ```
  "add node D to line 4"
  "add node F to line 4"
  "add node H to line 4"
  "add node G to line 4"
  "add node A to line 4"
  "4 D F H G A F H G A"
  "add node A to line 1"
  "add node B to line 1"
  "add node C to line 1"
  "add node D to line 1"
  "1 A B C D B C D"
  "4 D F H G A F H G A"
  "add node E to line 2"
  "add node B to line 2"
  "add node G to line 2"
  "1 A B C D B C D"
  "2 E B G B G"
  "4 D F H G A F H G A"
  ```



- ```c++
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
          qDebug() << QString("add node %1 to line %2").arg(name).arg(lineId);
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
              qDebug() << QString("add node %1 to line %2").arg(name).arg(lineId);
          }
      }
  
      return true;
  }
  ```

  ```c++
  void LineList::addNode(const QSharedPointer<StationNode>& node, const QString& preName)
  {
      // 插入链表的三种情况
      if (preName.isEmpty())
          m_line.push_front(node);
      else if (preName == end()->param.name)
          m_line.push_back(node);
  
      int index = 1;
      for (const auto& it : m_line) {
          if ((*it).param.name == preName) {
              m_line.insert(index, node);
              break;
          }
          else {
              index++;
          }
      }
  }
  ```



# 数组越界，程序异常退出

```
ASSERT failure in QVector<T>::operator[]: "index out of range", file D:/Qt/Qt5.14.2/5.14.2/mingw73_64/include/QtCore/qvector.h, line 459
22:37:07: The program has unexpectedly finished.
22:37:07: The process was ended forcefully.
```

```c++
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
        for (auto station = lineList.begin(); station != lineList.end(); ++station) {
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
            addNodeToGraph(node, multiStations);
        }
    }

    m_isBuildGraphSucess = true;
    return true;
}
```



# 邻接表结构错误

- 邻接表中的每个 edge 都被添加了两次

```
"B" QSharedPointer(0x96e3e0)
"H" QSharedPointer(0x96e620)
"A" QSharedPointer(0x96ea60)
"F" QSharedPointer(0x96e1a0)
"C" QSharedPointer(0x96eda0)
"D" QSharedPointer(0x96e0a0)
"E" QSharedPointer(0x96e0e0)
"G" QSharedPointer(0x96ed20)
"1 A B C D"
"2 E B G"
"4 D F H G A"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3) -> (E, 3) -> (E, 3) -> (G, 2) -> (G, 2)"
"H -> (F, 2) -> (F, 2) -> (G, 2) -> (G, 2)"
"A -> (B, 2) -> (B, 2) -> (G, 3) -> (G, 3)"
"F -> (D, 2) -> (D, 2) -> (H, 2) -> (H, 2)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
"D -> (C, 2) -> (C, 2) -> (F, 2) -> (F, 2)"
"E -> (B, 3) -> (B, 3)"
"G -> (B, 2) -> (B, 2) -> (H, 2) -> (H, 2) -> (A, 3) -> (A, 3)"
```



在 addNodeToGraph 中加入日志

```
"A -> (B, 2)"
"B -> (A, 2)"
 
"A -> (B, 2) -> (B, 2)"
"B -> (A, 2) -> (A, 2) -> (C, 3)"
"C -> (B, 3)"
 
"D -> (C, 2)"
"A -> (B, 2) -> (B, 2)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3)"
"C -> (B, 3) -> (B, 3) -> (D, 2)"
 
"D -> (C, 2) -> (C, 2)"
"A -> (B, 2) -> (B, 2)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
 
"D -> (C, 2) -> (C, 2)"
"A -> (B, 2) -> (B, 2)"
"E -> (B, 3)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3) -> (E, 3)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
 
"D -> (C, 2) -> (C, 2)"
"A -> (B, 2) -> (B, 2)"
"E -> (B, 3) -> (B, 3)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3) -> (E, 3) -> (E, 3) -> (G, 2)"
"G -> (B, 2)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
 
"D -> (C, 2) -> (C, 2)"
"A -> (B, 2) -> (B, 2)"
"E -> (B, 3) -> (B, 3)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3) -> (E, 3) -> (E, 3) -> (G, 2) -> (G, 2)"
"G -> (B, 2) -> (B, 2)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
 
"D -> (C, 2) -> (C, 2) -> (F, 2)"
"A -> (B, 2) -> (B, 2)"
"E -> (B, 3) -> (B, 3)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3) -> (E, 3) -> (E, 3) -> (G, 2) -> (G, 2)"
"G -> (B, 2) -> (B, 2)"
"F -> (D, 2)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
 
"H -> (F, 2)"
"D -> (C, 2) -> (C, 2) -> (F, 2) -> (F, 2)"
"A -> (B, 2) -> (B, 2)"
"E -> (B, 3) -> (B, 3)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3) -> (E, 3) -> (E, 3) -> (G, 2) -> (G, 2)"
"G -> (B, 2) -> (B, 2)"
"F -> (D, 2) -> (D, 2) -> (H, 2)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
 
"H -> (F, 2) -> (F, 2) -> (G, 2)"
"D -> (C, 2) -> (C, 2) -> (F, 2) -> (F, 2)"
"A -> (B, 2) -> (B, 2)"
"E -> (B, 3) -> (B, 3)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3) -> (E, 3) -> (E, 3) -> (G, 2) -> (G, 2)"
"G -> (B, 2) -> (B, 2) -> (H, 2)"
"F -> (D, 2) -> (D, 2) -> (H, 2) -> (H, 2)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
 
"H -> (F, 2) -> (F, 2) -> (G, 2) -> (G, 2)"
"D -> (C, 2) -> (C, 2) -> (F, 2) -> (F, 2)"
"A -> (B, 2) -> (B, 2) -> (G, 3)"
"E -> (B, 3) -> (B, 3)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3) -> (E, 3) -> (E, 3) -> (G, 2) -> (G, 2)"
"G -> (B, 2) -> (B, 2) -> (H, 2) -> (H, 2) -> (A, 3)"
"F -> (D, 2) -> (D, 2) -> (H, 2) -> (H, 2)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
 
"H -> (F, 2) -> (F, 2) -> (G, 2) -> (G, 2)"
"D -> (C, 2) -> (C, 2) -> (F, 2) -> (F, 2)"
"A -> (B, 2) -> (B, 2) -> (G, 3) -> (G, 3)"
"E -> (B, 3) -> (B, 3)"
"B -> (A, 2) -> (A, 2) -> (C, 3) -> (C, 3) -> (E, 3) -> (E, 3) -> (G, 2) -> (G, 2)"
"G -> (B, 2) -> (B, 2) -> (H, 2) -> (H, 2) -> (A, 3) -> (A, 3)"
"F -> (D, 2) -> (D, 2) -> (H, 2) -> (H, 2)"
"C -> (B, 3) -> (B, 3) -> (D, 2) -> (D, 2)"
```

```C++
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
        for (auto station = lineList.begin(); station != lineList.end(); ++station, ++station) { // 
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
            i += 2;  // 每次移动两次
            addNodeToGraph(node, multiStations);
        }
    }

    m_isBuildGraphSucess = true;
    return true;
}
```

通过

```
"E" QSharedPointer(0x30fe0e0)
"D" QSharedPointer(0x30feba0)
"F" QSharedPointer(0x30feb20)
"H" QSharedPointer(0x30feb60)
"A" QSharedPointer(0x30fe7e0)
"C" QSharedPointer(0x30fe0a0)
"G" QSharedPointer(0x30fe8a0)
"B" QSharedPointer(0x30fe120)
"1 A B C D"
"2 E B G"
"4 D F H G A"
"E -> (B, 3)"
"D -> (C, 2) -> (F, 2)"
"F -> (D, 2) -> (H, 2)"
"H -> (F, 2) -> (G, 2)"
"A -> (B, 2) -> (G, 3)"
"C -> (B, 3) -> (D, 2)"
"G -> (B, 2) -> (H, 2) -> (A, 3)"
"B -> (A, 2) -> (C, 3) -> (E, 3) -> (G, 2)"
```



# removeNode failed

- 现象

```
"remove node I failed as it does not belong to line 0"
```

- 源代码

```c++
bool SubwayGraph::removeNode(const QString &name, const MultiBiDirectionStations& biStations, QString& err_msg)
{
    if (m_stations.find(name) == m_stations.end()) {
        err_msg = QString("remove node %1 failed as it is not existed").arg(name);
        return false;
    }

    // 先从 Lines 中移除，然后从 Graph 中移除，最后从 Stations 中移除
    const QSharedPointer<StationNode>& node = m_stations[name];
    const QSet<int>& belongsLines = node->param.belongingLines;
    for (auto line = belongsLines.begin(); biStation != belongsLines.end(); ++belongsLines) {
        int lineId = *line;
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
```

- 修改

- ```C++
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
  ```

  

```
"I" QSharedPointer(0x179e5a0)
"A" QSharedPointer(0x179eba0)
"C" QSharedPointer(0x179df60)
"G" QSharedPointer(0x179e6e0)
"D" QSharedPointer(0x179e3a0)
"H" QSharedPointer(0x179e660)
"B" QSharedPointer(0x179e560)
"F" QSharedPointer(0x179e120)
"E" QSharedPointer(0x179ebe0)
"1 A B C D"
"2 I E B G"
"4 D F H G A I"
"I -> (E, 4) -> (A, 3)"
"A -> (B, 2) -> (G, 3) -> (I, 3)"
"C -> (B, 3) -> (D, 2)"
"G -> (B, 2) -> (H, 2) -> (A, 3)"
"D -> (C, 2) -> (F, 2)"
"H -> (F, 2) -> (G, 2)"
"B -> (A, 2) -> (C, 3) -> (E, 3) -> (G, 2)"
"F -> (D, 2) -> (H, 2)"
"E -> (B, 3) -> (I, 4)"
"A" QSharedPointer(0x179eba0)
"C" QSharedPointer(0x179df60)
"G" QSharedPointer(0x179e6e0)
"D" QSharedPointer(0x179e3a0)
"H" QSharedPointer(0x179e660)
"B" QSharedPointer(0x179e560)
"F" QSharedPointer(0x179e120)
"E" QSharedPointer(0x179ebe0)
"1 A B C D"
"2 E B G"
"4 D F H G A"
"A -> (B, 2) -> (G, 3)"
"C -> (B, 3) -> (D, 2)"
"G -> (B, 2) -> (H, 2) -> (A, 3)"
"D -> (C, 2) -> (F, 2)"
"H -> (F, 2) -> (G, 2)"
"B -> (A, 2) -> (C, 3) -> (E, 3) -> (G, 2)"
"F -> (D, 2) -> (H, 2)"
"E -> (B, 3)"
```



# JsonParser 解析的数据 build 失败

```c++
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
    }
    else {
        printSubwayGraph(subwayGraph);
    }
}
```

```
"build stations failed as add node to set failed"
```



- 日志

```c++
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
```

```
"build stations failed as add node to set failed"
"C B D"
"A B"
"2 3"
"3"
"A" 30 114 2
"B" 31 115 2
"C" 32 116 2
"D" 33 117 2
```

- JsonParser::parse 中构造 StationNodeParam 对象时参数传递顺序搞反，修改后成功 build

```
"1 A B"
"2 C B D"
"B -> (A, 3) -> (C, 2) -> (D, 3)"
"A -> (B, 3)"
"D -> (B, 3)"
"C -> (B, 2)"
```



# 进程崩溃

- 现象

- ```
  22:57:05: The process was ended forcefully.
  ```

  ```C++
  void HttpResponseHandler::schedulePendingHttpRequest(const QSet<QUrl> &pendingUrls, int interval)
  {
      QTimer* timer = new QTimer(this);
      timer->setInterval(interval);
  
      // use lambda expression to capture pendingUrls and update progress easier
      auto url = pendingUrls.begin();
      (void)connect(timer, &QTimer::timeout, this, [&](){
          if (url != pendingUrls.end()) {
              emit httpRequestPending(*url);
              ++url;
          }
          else {
              timer->stop();
          }
      });
  
      timer->start();
  }
  
  bool WhAppHttpResponseParser::parse(const QString& context, QString &err_msg)
  {
      HttpResponseHandler::SubwayLines& subwayLines = m_handler.subwayLines();
  
      // 匹配每个线路的块
      QRegularExpression lineBlockRegex(
          R"(<div class="line-list">(.*?)<div class="clearfix"></div>\s*</div>\s*</div>\s*</div>)",
          QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
      );
  
      // 匹配线路编号（从标题中提取）
      QRegularExpression lineNumberRegex(
          R"(武汉地铁(\d+)号线线路图)"
      );
  
      // 改进后的站点正则：允许class="link"属性位置变化，并确保跨行匹配
      QRegularExpression stationRegex(
          R"(<div class="station">[\s\S]*?<a\s+[^>]*?class="link"[^>]*>([^<]+)</a>)",
          QRegularExpression::CaseInsensitiveOption
      );
  
      // 遍历所有线路块
      QRegularExpressionMatchIterator blockIter = lineBlockRegex.globalMatch(context);
      while (blockIter.hasNext()) {
          QRegularExpressionMatch blockMatch = blockIter.next();
          QString blockContent = blockMatch.captured(1);
  
          // 提取线路编号
          QRegularExpressionMatch lineNumberMatch = lineNumberRegex.match(blockContent);
          if (!lineNumberMatch.hasMatch()) continue;
          int lineNumber = lineNumberMatch.captured(1).toInt();
  
          // 提取站点名称
          QVector<QString> stations;
          QRegularExpressionMatchIterator stationIter = stationRegex.globalMatch(blockContent);
          while (stationIter.hasNext()) {
              QRegularExpressionMatch stationMatch = stationIter.next();
              QString stationName = stationMatch.captured(1).trimmed();
              stations.append(stationName);
          }
  
          // 存入结果
          subwayLines.insert(lineNumber, stations);
      }
  
      // extract deatil url
      ...
  
      // request bendibao detail page
      ...
  
      // request baidu map
      QSet<QUrl> pendingUrls;
      QSet<QString> stations;
      for (auto line = subwayLines.begin(); line != subwayLines.end(); ++line) {
          for (const auto& station : line.value()) {
              if (stations.find(station) != stations.end()) {
                  continue;
              }
              stations.insert(station);
              const QUrl& url = BDMapHttpResponseParser::generateBDMapRequestUrl(station);
              pendingUrls.insert(url);
          }
      }
      m_handler.schedulePendingHttpRequest(pendingUrls, BDMap_Request_Interval);
  
      return true;
  }
  ```



- 注释该函数后不会崩溃

  ```c++
  // m_handler.schedulePendingHttpRequest(pendingUrls, BDMap_Request_Interval);
  ```

- 输出结果

- ```
  23:07:22: Starting D:\Qt_Projects\SubwayTransferSystem\build-SubwayTransferSystem-Desktop_Qt_5_14_2_MinGW_64_bit-Debug\debug\SubwayTransferSystem.exe ...
  "https://api.map.baidu.com/geocoding/v3/?address=五环体育中心&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=鼓架山站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=集贤&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=武汉商务区&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=园博园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=省博湖北日报&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=青鱼嘴&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=常青城&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=取水楼&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=小洪山&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=十里铺&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=古田一路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=五里墩&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=赵家条&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=汉阳客运站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=新城十一路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=复兴路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=码头潭公园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=水果湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光谷四路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=通航机场&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=洪山路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=马房山&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=洪山广场&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=武汉站西广场站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光谷五路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=六渡桥&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=汉阳火车站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=小军山&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=天河机场&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=钟家村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=长港路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=青年路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=小龟山&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=新农&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=径河&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=新路村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=建设二路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=广埠屯&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=中医药大学&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=大花岭&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=三角路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=友谊路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=古田三路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=唐家墩&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=琴台&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=航空总部&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=罗家港&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=菱角湖路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=循礼门&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=中山公园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=建港&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=蔡甸广场&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=体育中心&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=昙华林武胜门&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=香港路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=杨春湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=大智路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=二七路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=北华街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=花山新城站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=二七小路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=国博中心南&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=马池&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=文治街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=东亭&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=二雅路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=红钢城&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=湖口&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=左岭&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=苗栗路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=沌口&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=厂前&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=塔子湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=长岭山&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=湖北大学&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=三店&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=华中科技大学&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=国博中心北&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=汉西一路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=楚河汉街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=柏林&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=王家墩东&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=三角湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=汪家墩&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=汤云海路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=板桥&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=竹叶山&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=金银潭&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=梅苑小区&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=司门口黄鹤楼&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=青龙山地铁小镇&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=武东站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光谷大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=三层楼&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=云飞路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=武钢&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=武胜路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=青宜居&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=大军山&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=马影河&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=罗家庄&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=兴业路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=湾湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=江夏客厅&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=新月溪公园站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=岳家嘴&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=双墩&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=文昌路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=八铺街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=工业四路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=园林路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=街道口&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=老关村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=惠济二路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=利济北路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=滕子岗&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=市民之家&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=古田二路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=杨家湾&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=石桥&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=东吴大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=藏龙东街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=梨园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=马湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=南太子湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=谭鑫培公园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=黄家湖地铁小镇&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=轻工大学&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=后湖大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=工人村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=常青花园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=玉龙路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=马鹦路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=孟家铺&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=头道街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=太平洋&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=桂子湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=珞雄路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=彭刘杨路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=余家头&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光谷生物园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=五环大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=豹澥&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=三眼桥&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=范湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=瑞安街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光谷六路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=中一路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=和平公园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=佳园路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=腾龙大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=竹叶海&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=花山河站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=军运村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=金融港北&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=巨龙大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光谷七路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=常码头&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=新庙村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光谷广场&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=未来一路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=张家湾&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=首义路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=虎泉&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=金银湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=硚口路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=枫林站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=葛店南&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=海口三路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=小东门&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=武昌火车站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=仁和路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=汉口火车站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=佛祖岭&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=七里庙&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=黄龙山路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=车城东路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=科普公园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=堤角&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=古田四路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=园博园北&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=杨园铁四院&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光谷同济医院&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=江汉路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=滠口新城&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=徐东&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=王家湾&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=中南医院&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=前进村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=宗关&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=野芷湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=周家河&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=武汉站东广场&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=宋家岗&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=金潭路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=临嶂大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=东风公司&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=中南路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=知音&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=白沙六路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=铁机路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=宝通寺&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=烽火村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=凤凰路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=武汉火车站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=秀湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=红霞&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=宏图大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=额头湾&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=新荣&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光谷五路站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=拦江路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=湖工大&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=杨汊湖&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=裕福路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=纱帽&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=陶家岭&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=黄家湖（武科大）&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=江城大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=省农科院&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=崇仁路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=螃蟹岬&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=徐州新村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=黄金口&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=龙阳村&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=四新大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=天阳大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=汉口北&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=盘龙城&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=永安堂&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=沌阳大道&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=舵落口&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=建安街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=积玉桥&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=丹水池&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=徐家棚&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=新河街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=协子河&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=横店&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=未来三路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=金银湖公园&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=光霞&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=新天&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=三阳路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=纸坊大街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=汉正街&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=黄浦路&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "https://api.map.baidu.com/geocoding/v3/?address=武汉东站&output=json&ak=gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu"
  "5 : 红霞 黄家湖（武科大） 中医药大学 白沙六路 光霞 张家湾 烽火村 八铺街 复兴路 彭刘杨路 司门口黄鹤楼 昙华林武胜门 积玉桥 三层楼 三角路 徐家棚 杨园铁四院 余家头 科普公园 建设二路 和平公园 红钢城 青宜居 工人村 武钢 厂前 武汉站东广场"
  "4 : 武汉火车站 杨春湖 工业四路 仁和路 园林路 罗家港 铁机路 岳家嘴 东亭 青鱼嘴 楚河汉街 洪山广场 中南路 梅苑小区 武昌火车站 首义路 复兴路 拦江路 钟家村 汉阳火车站 五里墩 七里庙 十里铺 王家湾 玉龙路 永安堂 孟家铺 黄金口 新天 集贤 知音 新农 凤凰路 蔡甸广场 临嶂大道 新庙村 柏林"
  "7 : 横店 裕福路 天阳大道 腾龙大道 巨龙大道 汤云海路 马池 园博园北 园博园 常码头 武汉商务区 王家墩东 取水楼 香港路 三阳路 徐家棚 湖北大学 新河街 螃蟹岬 小东门 武昌火车站 瑞安街 建安街 湖工大 板桥 野芷湖 新路村 大花岭 江夏客厅 谭鑫培公园 北华街 纸坊大街 青龙山地铁小镇"
  "6 : 新城十一路 码头潭公园 五环体育中心 二雅路 海口三路 金银湖公园 金银湖 园博园北 轻工大学 常青花园 杨汊湖 石桥 唐家墩 三眼桥 香港路 苗栗路 大智路 江汉路 六渡桥 汉正街 武胜路 琴台 钟家村 马鹦路 建港 前进村 国博中心北 国博中心南 老关村 江城大道 车城东路 东风公司"
  "1 : 汉口北 滠口新城 滕子岗 堤角 新荣 丹水池 徐州新村 二七路 头道街 黄浦路 三阳路 大智路 循礼门 友谊路 利济北路 崇仁路 硚口路 太平洋 宗关 汉西一路 古田四路 古田三路 古田二路 古田一路 舵落口 竹叶海 额头湾 五环大道 东吴大道 码头潭公园 三店 径河"
  "16 : 国博中心南 老关村 南太子湖 沌口 小军山 枫林站 大军山 桂子湖 马影河 协子河 湾湖 周家河 纱帽 通航机场"
  "19 : 武汉站西广场站 武东站 鼓架山站 花山新城站 花山河站 光谷五路站 新月溪公园站"
  "3 : 宏图大道 市民之家 后湖大道 兴业路 二七小路 罗家庄 赵家条 惠济二路 香港路 菱角湖路 范湖 云飞路 武汉商务区 双墩 宗关 王家湾 龙阳村 陶家岭 四新大道 汉阳客运站 三角湖 体育中心 东风公司 沌阳大道"
  "2 : 天河机场 航空总部 宋家岗 巨龙大道 盘龙城 宏图大道 常青城 金银潭 常青花园 长港路 汉口火车站 范湖 王家墩东 青年路 中山公园 循礼门 江汉路 积玉桥 螃蟹岬 小龟山 洪山广场 中南路 宝通寺 街道口 广埠屯 虎泉 杨家湾 光谷广场 珞雄路 华中科技大学 光谷大道 佳园路 武汉东站 黄龙山路 金融港北 秀湖 藏龙东街 佛祖岭"
  "8 : 金潭路 宏图大道 塔子湖 中一路 竹叶山 赵家条 黄浦路 徐家棚 徐东 汪家墩 岳家嘴 梨园 省博湖北日报 中南医院 水果湖 洪山路 小洪山 街道口 马房山 文治街 文昌路 省农科院 马湖 野芷湖 黄家湖地铁小镇 军运村"
  "11 : 武汉东站 湖口 光谷同济医院 光谷生物园 光谷四路 光谷五路 光谷六路 豹澥 光谷七路 长岭山 未来一路 未来三路 左岭 葛店南"
  "东风公司"   130.411   46.8284
  23:08:13: D:\Qt_Projects\SubwayTransferSystem\build-SubwayTransferSystem-Desktop_Qt_5_14_2_MinGW_64_bit-Debug\debug\SubwayTransferSystem.exe exited with code 0
  ```


- 怀疑是 `void HttpResponseHandler::schedulePendingHttpRequest(const QSet<QUrl>& pendingUrls)` 中的形参 `pendingUrls` 是临时对象的引用，对象析构后 pendingUrls 成为悬空引用引发段错误。将其在函数内拷贝一份 `m_pendingUrls = pendingUrls;` 结果仍然报错。

- 报错函数为 qhash.h 文件的自增运算符重载函数，怀疑在 lambda expression  中通过引用捕获使用临时迭代器导致段错误。使用成员变量 `QSet<QUrl>::const_iterator m_urlIndex;`遍历 `pendingUrls`

- 结果正确

- ```C++
  void HttpResponseHandler::schedulePendingHttpRequest(const QSet<QUrl>& pendingUrls, int interval)
  {
      QTimer* timer = new QTimer(this);
      timer->setInterval(interval);
  
      // use lambda expression to capture pendingUrls and update progress easier
      m_pendingUrls = pendingUrls;
      m_urlIndex = m_pendingUrls.begin();
      (void)connect(timer, &QTimer::timeout, this, [this, timer](){
          if (m_urlIndex != m_pendingUrls.end()) {
              emit httpRequestPending(*m_urlIndex);
              ++m_urlIndex;
          }
          else {
              timer->stop();
              timer->deleteLater();
          }
      });
  
      timer->start();
  }
  ```

- 问题总结：在 lambda expreesion 中通过引用捕获使用了临时对象，函数执行结束后临时对象析构，lambda expreesion 再次访问导致段错误



# 百度地图返回经纬度不准

- ```
  "东风公司"   130.411   46.8284
  "横店"   114.306   30.8088
  "东吴大道"   0   0
  "舵落口"   114.176   30.6155
  "新庙村"   104.293   30.3975
  "光谷大道"   0   0
  "左岭"   114.617   30.531
  "汉阳火车站"   114.225   30.5601
  "葛店南"   115.026   33.7619
  "滠口新城"   114.349   30.6903
  "常青花园"   114.249   30.6472
  "利济北路"   0   0
  "市民之家"   120.197   30.4798
  "红霞"   114.105   22.5731
  "杨春湖"   114.423   30.61
  "广埠屯"   114.372   30.5309
  "武汉火车站"   114.312   30.5985
  "车城东路"   0   0
  "孟家铺"   118.807   39.3459
  "八铺街"   0   0
  "汉西一路"   0   0
  "香港路"   0   0
  "金潭路"   0   0
  "龙阳村"   114.211   30.5599
  "玉龙路"   0   0
  "新月溪公园站"   116.241   39.8516
  "汤云海路"   0   0
  "二七小路"   0   0
  "三层楼"   106.25   29.1984
  "徐东"   114.354   30.5943
  "国博中心南"   106.552   29.7227
  "友谊路"   0   0
  "协子河"   119.647   33.43
  "湖工大"   114.32   30.489
  "滕子岗"   114.348   30.6808
  "竹叶海"   114.17   30.6202
  "园林路"   0   0
  "天河机场"   113.409   23.1286
  "体育中心"   117.184   39.0735
  "昙华林武胜门"   106.302   30.3555
  "军运村"   114.299   30.4315
  "码头潭公园"   113.243   23.1975
  "青宜居"   114.427   30.6496
  "武汉东站"   114.437   30.4921
  "花山新城站"   118.5   31.7255
  "武汉站东广场"   114.433   30.6136
  "古田三路"   0   0
  "新城十一路"   0   0
  "徐州新村"   117.018   34.5634
  "光谷四路"   0   0
  "三阳路"   0   0
  "取水楼"   114.278   30.598
  "赵家条"   114.304   30.6237
  "光谷五路"   0   0
  "小龟山"   114.333   30.557
  "未来一路"   0   0
  "塔子湖"   114.287   30.6585
  "马鹦路"   0   0
  "新路村"   121.839   30.9798
  "野芷湖"   114.348   30.4717
  "竹叶山"   114.292   30.6275
  "未来三路"   0   0
  "青鱼嘴"   114.356   30.5702
  "大智路"   0   0
  "新农"   105.356   31.2384
  "凤凰路"   0   0
  "古田四路"   0   0
  "盘龙城"   114.277   30.7287
  "光谷生物园"   114.548   30.4843
  "光霞"   114.3   30.4836
  "瑞安街"   0   0
  "新荣"   113.146   40.262
  "前进村"   121.759   31.1245
  "新天"   100.593   38.5389
  "菱角湖路"   0   0
  "黄家湖（武科大）"   120.562   31.0249
  "谭鑫培公园"   114.33   30.3754
  "螃蟹岬"   114.332   30.6623
  "天阳大道"   0   0
  "光谷七路"   0   0
  "中一路"   0   0
  "古田二路"   0   0
  "和平公园"   117.128   40.1611
  "东风公司"   130.411   46.8284
  "金银潭"   114.285   30.6642
  "杨园铁四院"   114.344   30.6065
  "小军山"   114.176   30.4241
  "湾湖"   115.093   26.3102
  "板桥"   121.467   25.0093
  "建设二路"   0   0
  "四新大道"   0   0
  "光谷五路站"   114.611   30.5419
  "豹澥"   114.549   30.4852
  "南太子湖"   115.188   30.0346
  "马房山"   114.356   30.521
  "省农科院"   104.114   30.6301
  "云飞路"   0   0
  "北华街"   0   0
  "航空总部"   114.234   30.7392
  "东亭"   120.374   31.587
  "海口三路"   0   0
  "金银湖"   114.228   30.6558
  "唐家墩"   120.123   30.4738
  "周家河"   103.294   30.492
  "杨汊湖"   114.268   30.6414
  "复兴路"   0   0
  "集贤"   131.147   46.7345
  "三店"   114.8   33.1808
  "马湖"   117.841   32.0784
  "堤角"   114.343   30.662
  "街道口"   116.689   39.8859
  "梨园"   0   0
  "司门口黄鹤楼"   114.307   30.5527
  "范湖"   113.691   33.8493
  "大军山"   114.091   30.4116
  "王家湾"   114.857   40.2716
  "科普公园"   116.581   39.7046
  "汉正街"   0   0
  "余家头"   119.615   30.3107
  "桂子湖"   114.098   30.3625
  "长岭山"   123.98   44.2898
  "马池"   114.242   30.6643
  "循礼门"   114.29   30.5918
  "大花岭"   116.116   40.4116
  "武东站"   125.37   43.8986
  "彭刘杨路"   0   0
  "楚河汉街"   0   0
  "金融港北"   114.427   30.4676
  "永安堂"   116.423   39.9485
  "柏林"   104.739   23.2415
  "轻工大学"   114.241   30.6459
  "枫林站"   115.416   29.757
  "花山河站"   114.519   30.5471
  "沌阳大道"   0   0
  "水果湖"   114.349   30.5604
  "杨家湾"   104.98   27.18
  "光谷六路"   0   0
  "知音"   116.591   39.8089
  "文治街"   0   0
  "沌口"   114.158   30.4957
  "秀湖"   114.428   30.4534
  "江城大道"   0   0
  "江汉路"   0   0
  "光谷同济医院"   114.611   30.5419
  "七里庙"   114.224   30.4778
  "徐家棚"   114.349   30.587
  "工人村"   123.331   41.7973
  "崇仁路"   0   0
  "五环大道"   0   0
  "青龙山地铁小镇"   121.046   42.4011
  "文昌路"   0   0
  "园博园"   116.197   39.8817
  "三角路"   0   0
  "琴台"   112.911   33.7457
  "藏龙东街"   0   0
  "额头湾"   114.163   30.6239
  "汉口北"   114.322   30.7054
  "中南路"   0   0
  "小洪山"   105.818   29.8834
  "三角湖"   114.172   30.5162
  "光谷广场"   114.404   30.5115
  "梅苑小区"   116.636   40.3399
  "巨龙大道"   0   0
  "汪家墩"   119.875   30.3514
  "汉口火车站"   114.262   30.6216
  "武汉站西广场站"   114.431   30.6128
  "金银湖公园"   114.176   30.6604
  "二雅路"   0   0
  "武汉商务区"   114.253   30.6042
  "十里铺"   107.204   34.374
  "新河街"   0   0
  "黄金口"   121.544   31.2235
  "宝通寺"   116.21   39.8091
  "石桥"   120.2   30.3382
  "青年路"   0   0
  "马影河"   114.039   30.3211
  "腾龙大道"   0   0
  "建安街"   0   0
  "双墩"   117.271   32.0137
  "硚口路"   0   0
  "白沙六路"   0   0
  "罗家庄"   114.552   22.4979
  "惠济二路"   0   0
  "宋家岗"   106.747   29.6606
  "五里墩"   114.077   32.1334
  "建港"   121.848   31.0365
  "头道街"   0   0
  "珞雄路"   0   0
  "积玉桥"   0   0
  "黄浦路"   0   0
  "烽火村"   103.793   30.3597
  "厂前"   114.443   30.6206
  "铁机路"   0   0
  "苗栗路"   0   0
  "首义路"   0   0
  "钟家村"   121.224   31.3827
  "拦江路"   0   0
  "岳家嘴"   114.368   30.5842
  "洪山广场"   114.343   30.5514
  "佛祖岭"   114.458   30.4639
  "纸坊大街"   0   0
  "兴业路"   0   0
  "径河"   114.171   30.6844
  "蔡甸广场"   114.049   30.5797
  "裕福路"   0   0
  "红钢城"   114.419   30.6517
  "常码头"   114.244   30.6177
  "三眼桥"   0   0
  "中山公园"   116.401   39.9167
  "张家湾"   114.286   30.4886
  "丹水池"   114.341   30.6492
  "华中科技大学"   120.947   31.4431
  "佳园路"   0   0
  "宏图大道"   0   0
  "常青城"   113.146   41.033
  "洪山路"   0   0
  "园博园北"   116.197   39.8817
  "中医药大学"   116.439   39.8702
  "纱帽"   114.092   30.32
  "省博湖北日报"   114.369   30.5714
  "湖北大学"   114.336   30.5837
  "罗家港"   120.498   31.0105
  "黄家湖地铁小镇"   116.145   39.7506
  "武钢"   114.489   30.4594
  "工业四路"   0   0
  "汉阳客运站"   114.225   30.5601
  "仁和路"   0   0
  "长港路"   0   0
  "武胜路"   0   0
  "临嶂大道"   0   0
  "二七路"   0   0
  "太平洋"   116.516   39.8051
  "通航机场"   106.533   29.5323
  "国博中心北"   106.552   29.7227
  "六渡桥"   0   0
  "老关村"   105.7   30.2514
  "古田一路"   0   0
  "陶家岭"   105.672   30.0775
  "五环体育中心"   117.184   39.0735
  "小东门"   121.508   31.2257
  "湖口"   116.258   29.7371
  "鼓架山站"   117.322   31.1592
  "虎泉"   114.376   30.5192
  "王家墩东"   121.557   30.8789
  "后湖大道"   0   0
  "宗关"   114.236   30.5851
  "黄龙山路"   0   0
  "武昌火车站"   0   0
  "中南医院"   109.411   24.3232
  "江夏客厅"   114.328   30.395
  ```

- 怀疑是地点名称不精准，调整百度地图请求url，将 address 填写为全称武汉市xxx地铁站，如：武汉市循礼门地址站

- 结果正确



# print StationInfos 无数据

```
11 : 武汉东站 湖口 光谷同济医院 光谷生物园 光谷四路 光谷五路 光谷六路 豹澥 光谷七路 长岭山 未来一路 未来三路 左岭 葛店南"
"8 : 金潭路 宏图大道 塔子湖 中一路 竹叶山 赵家条 黄浦路 徐家棚 徐东 汪家墩 岳家嘴 梨园 省博湖北日报 中南医院 水果湖 洪山路 小洪山 街道口 马房山 文治街 文昌路 省农科院 马湖 野芷湖 黄家湖地铁小镇 军运村"
"19 : 武汉站西广场站 武东站 鼓架山站 花山新城站 花山河站 光谷五路站 新月溪公园站"
"3 : 宏图大道 市民之家 后湖大道 兴业路 二七小路 罗家庄 赵家条 惠济二路 香港路 菱角湖路 范湖 云飞路 武汉商务区 双墩 宗关 王家湾 龙阳村 陶家岭 四新大道 汉阳客运站 三角湖 体育中心 东风公司 沌阳大道"
"2 : 天河机场 航空总部 宋家岗 巨龙大道 盘龙城 宏图大道 常青城 金银潭 常青花园 长港路 汉口火车站 范湖 王家墩东 青年路 中山公园 循礼门 江汉路 积玉桥 螃蟹岬 小龟山 洪山广场 中南路 宝通寺 街道口 广埠屯 虎泉 杨家湾 光谷广场 珞雄路 华中科技大学 光谷大道 佳园路 武汉东站 黄龙山路 金融港北 秀湖 藏龙东街 佛祖岭"
"1 : 汉口北 滠口新城 滕子岗 堤角 新荣 丹水池 徐州新村 二七路 头道街 黄浦路 三阳路 大智路 循礼门 友谊路 利济北路 崇仁路 硚口路 太平洋 宗关 汉西一路 古田四路 古田三路 古田二路 古田一路 舵落口 竹叶海 额头湾 五环大道 东吴大道 码头潭公园 三店 径河"
"16 : 国博中心南 老关村 南太子湖 沌口 小军山 枫林站 大军山 桂子湖 马影河 协子河 湾湖 周家河 纱帽 通航机场"
"7 : 横店 裕福路 天阳大道 腾龙大道 巨龙大道 汤云海路 马池 园博园北 园博园 常码头 武汉商务区 王家墩东 取水楼 香港路 三阳路 徐家棚 湖北大学 新河街 螃蟹岬 小东门 武昌火车站 瑞安街 建安街 湖工大 板桥 野芷湖 新路村 大花岭 江夏客厅 谭鑫培公园 北华街 纸坊大街 青龙山地铁小镇"
"6 : 新城十一路 码头潭公园 五环体育中心 二雅路 海口三路 金银湖公园 金银湖 园博园北 轻工大学 常青花园 杨汊湖 石桥 唐家墩 三眼桥 香港路 苗栗路 大智路 江汉路 六渡桥 汉正街 武胜路 琴台 钟家村 马鹦路 建港 前进村 国博中心北 国博中心南 老关村 江城大道 车城东路 东风公司"
"5 : 红霞 黄家湖（武科大） 中医药大学 白沙六路 光霞 张家湾 烽火村 八铺街 复兴路 彭刘杨路 司门口黄鹤楼 昙华林武胜门 积玉桥 三层楼 三角路 徐家棚 杨园铁四院 余家头 科普公园 建设二路 和平公园 红钢城 青宜居 工人村 武钢 厂前 武汉站东广场"
"4 : 武汉火车站 杨春湖 工业四路 仁和路 园林路 罗家港 铁机路 岳家嘴 东亭 青鱼嘴 楚河汉街 洪山广场 中南路 梅苑小区 武昌火车站 首义路 复兴路 拦江路 钟家村 汉阳火车站 五里墩 七里庙 十里铺 王家湾 玉龙路 永安堂 孟家铺 黄金口 新天 集贤 知音 新农 凤凰路 蔡甸广场 临嶂大道 新庙村 柏林"
"武汉市唐家墩地铁站" 114.278 30.6236 248
"武汉市前进村地铁站" 114.263 30.5295 247
"武汉市武汉站东广场地铁站" 114.433 30.6136 246
"武汉市谭鑫培公园地铁站" 114.33 30.3754 245
"武汉市径河地铁站" 114.127 30.6654 244
"武汉市二雅路地铁站" 114.157 30.6313 243
"武汉市金银湖公园地铁站" 114.176 30.6604 242
"武汉市罗家港地铁站" 114.378 30.5992 241
"武汉市古田三路地铁站" 114.211 30.6015 240
"武汉市武汉东站地铁站" 114.437 30.4921 239
"武汉市七里庙地铁站" 114.238 30.5627 238
"武汉市省农科院地铁站" 114.328 30.4903 237
"武汉市省博湖北日报地铁站" 114.369 30.5714 236
"武汉市湾湖地铁站" 113.839 30.3853 235
"武汉市后湖大道地铁站" 114.325 30.6614 234
"武汉市小东门地铁站" 114.323 30.5496 233
"武汉市凤凰路地铁站" 114.315 30.4935 232
"武汉市钟家村地铁站" 114.273 30.5553 231
"武汉市友谊路地铁站" 114.407 30.625 230
"武汉市珞雄路地铁站" 114.413 30.5081 229
"武汉市岳家嘴地铁站" 114.369 30.5834 228
"武汉市王家湾地铁站" 114.218 30.5714 227
"武汉市范湖地铁站" 114.234 30.6198 226
"武汉市五环体育中心地铁站" 114.15 30.6464 225
"武汉市裕福路地铁站" 114.312 30.5985 224
"武汉市腾龙大道地铁站" 114.283 30.727 223
"武汉市小洪山地铁站" 114.356 30.5453 222
"武汉市光谷五路地铁站" 114.509 30.4595 221
"武汉市瑞安街地铁站" 114.325 30.5186 220
"武汉市滕子岗地铁站" 114.312 30.5985 219
"武汉市水果湖地铁站" 114.353 30.5585 218
"武汉市中医药大学地铁站" 114.277 30.4538 217
"武汉市青年路地铁站" 114.273 30.5874 216
"武汉市二七路地铁站" 114.319 30.6335 215
"武汉市复兴路地铁站" 114.307 30.5304 214
"武汉市盘龙城地铁站" 114.266 30.7064 213
"武汉市江城大道地铁站" 114.23 30.5304 212
"武汉市昙华林武胜门地铁站" 114.312 30.5596 211
"武汉市青宜居地铁站" 114.427 30.6496 210
"武汉市野芷湖地铁站" 114.348 30.4717 209
"武汉市宝通寺地铁站" 114.347 30.538 208
"武汉市老关村地铁站" 114.225 30.5189 207
"武汉市汤云海路地铁站" 114.255 30.7058 206
"武汉市余家头地铁站" 114.361 30.6154 205
"武汉市鼓架山站地铁站" 114.483 30.565 204
"武汉市马房山地铁站" 114.356 30.521 203
"武汉市陶家岭地铁站" 114.208 30.5514 202
"武汉市赵家条地铁站" 114.304 30.6237 201
"武汉市罗家庄地铁站" 114.31 30.6291 200
"武汉市航空总部地铁站" 114.234 30.7392 199
"武汉市码头潭公园地铁站" 114.133 30.6422 198
"武汉市汉口火车站地铁站" 114.262 30.6216 197
"武汉市双墩地铁站" 114.239 30.593 196
"武汉市十里铺地铁站" 114.312 30.5985 195
"武汉市铁机路地铁站" 114.356 30.6127 194
"武汉市科普公园地铁站" 114.37 30.6233 193
"武汉市汉西一路地铁站" 114.227 30.5922 192
"武汉市纱帽地铁站" 114.092 30.32 191
"武汉市永安堂地铁站" 114.194 30.5711 190
"武汉市佳园路地铁站" 114.433 30.4921 189
"武汉市文治街地铁站" 114.341 30.508 188
"武汉市建港地铁站" 114.261 30.5274 187
"武汉市天阳大道地铁站" 114.312 30.5985 186
"武汉市竹叶山地铁站" 114.292 30.6275 185
"武汉市梨园地铁站" 114.376 30.581 184
"武汉市巨龙大道地铁站" 114.295 30.7085 183
"武汉市纸坊大街地铁站" 114.306 30.3465 182
"武汉市拦江路地铁站" 114.276 30.5487 181
"武汉市光谷同济医院地铁站" 114.472 30.4944 180
"武汉市金潭路地铁站" 114.286 30.6834 179
"武汉市五环大道地铁站" 114.145 30.6193 178
"武汉市东风公司地铁站" 114.172 30.5087 177
"武汉市新城十一路地铁站" 114.118 30.6333 176
"武汉市花山河站地铁站" 114.519 30.5471 175
"武汉市光谷生物园地铁站" 114.548 30.4843 174
"武汉市花山新城站地铁站" 114.496 30.5656 173
"武汉市楚河汉街地铁站" 114.345 30.5658 172
"武汉市东吴大道地铁站" 114.162 30.629 171
"武汉市杨春湖地铁站" 114.423 30.611 170
"武汉市横店地铁站" 114.306 30.8088 169
"武汉市司门口黄鹤楼地铁站" 114.307 30.5527 168
"武汉市塔子湖地铁站" 114.287 30.6585 167
"武汉市中南路地铁站" 114.34 30.5454 166
"武汉市车城东路地铁站" 114.184 30.4902 165
"武汉市汪家墩地铁站" 114.357 30.5915 164
"武汉市东亭地铁站" 114.366 30.5728 163
"武汉市园博园地铁站" 114.222 30.6322 162
"武汉市玉龙路地铁站" 114.21 30.5773 161
"武汉市江汉路地铁站" 114.292 30.5913 160
"武汉市四新大道地铁站" 114.208 30.534 159
"武汉市汉正街地铁站" 114.275 30.5708 158
"武汉市湖口地铁站" 116.258 29.7371 157
"武汉市小军山地铁站" 114.176 30.4241 156
"武汉市五里墩地铁站" 114.247 30.5602 155
"武汉市大军山地铁站" 114.091 30.4116 154
"武汉市取水楼地铁站" 114.278 30.598 153
"武汉市三角湖地铁站" 114.186 30.5222 152
"武汉市武胜路地铁站" 114.275 30.5767 151
"武汉市孟家铺地铁站" 114.312 30.5985 150
"武汉市王家墩东地铁站" 114.312 30.5985 149
"武汉市武昌火车站地铁站" 114.325 30.5427 148
"武汉市湖北大学地铁站" 114.336 30.5837 147
"武汉市黄家湖（武科大）地铁站" 114.347 30.5218 146
"武汉市武汉火车站地铁站" 114.312 30.5985 145
"武汉市徐东地铁站" 114.354 30.5943 144
"武汉市三角路地铁站" 114.331 30.5844 143
"武汉市天河机场地铁站" 114.236 30.8077 142
"武汉市轻工大学地铁站" 114.241 30.6459 141
"武汉市光谷五路站地铁站" 114.509 30.4595 140
"武汉市板桥地铁站" 114.32 30.48 139
"武汉市园林路地铁站" 114.243 30.9414 138
"武汉市华中科技大学地铁站" 114.267 30.5887 137
"武汉市太平洋地铁站" 114.247 30.5804 136
"武汉市梅苑小区地铁站" 114.334 30.5379 135
"武汉市沌口地铁站" 114.158 30.4957 134
"武汉市徐州新村地铁站" 114.329 30.6373 133
"武汉市体育中心地铁站" 114.174 30.6896 132
"武汉市三店地铁站" 114.312 30.5985 131
"武汉市滠口新城地铁站" 114.349 30.6903 130
"武汉市国博中心南地铁站" 114.248 30.5132 129
"武汉市二七小路地铁站" 114.312 30.5985 128
"武汉市光谷四路地铁站" 114.495 30.4598 127
"武汉市黄浦路地铁站" 114.307 30.6331 126
"武汉市金银潭地铁站" 114.271 30.6661 125
"武汉市柏林地铁站" 113.992 30.5893 124
"武汉市武东站地铁站" 114.437 30.4921 123
"武汉市光霞地铁站" 114.3 30.4836 122
"武汉市洪山广场地铁站" 114.343 30.5514 121
"武汉市工业四路地铁站" 114.314 30.4845 120
"武汉市头道街地铁站" 114.324 30.6239 119
"武汉市周家河地铁站" 114.874 30.8425 118
"武汉市海口三路地铁站" 114.172 30.6455 117
"武汉市集贤地铁站" 131.255 46.8217 116
"武汉市龙阳村地铁站" 114.211 30.5599 115
"武汉市汉口北地铁站" 114.322 30.7054 114
"武汉市常青城地铁站" 114.261 30.6667 113
"武汉市通航机场地铁站" 114.312 30.5985 112
"武汉市大花岭地铁站" 114.319 30.417 111
"武汉市宋家岗地铁站" 114.239 30.7316 110
"武汉市宏图大道地铁站" 114.286 30.6718 109
"武汉市黄金口地铁站" 114.044 30.4789 108
"武汉市循礼门地铁站" 114.29 30.5918 107
"武汉市建安街地铁站" 114.309 30.5113 106
"武汉市石桥地铁站" 114.329 30.3401 105
"武汉市湖工大地铁站" 114.32 30.4896 104
"武汉市左岭地铁站" 114.617 30.531 103
"武汉市红霞地铁站" 114.268 30.4366 102
"武汉市和平公园地铁站" 114.392 30.6396 101
"武汉市惠济二路地铁站" 114.296 30.6199 100
"武汉市新路村地铁站" 114.321 30.4508 99
"武汉市新庙村地铁站" 114.019 30.5903 98
"武汉市宗关地铁站" 114.236 30.5851 97
"武汉市武钢地铁站" 114.471 30.6015 96
"武汉市桂子湖地铁站" 114.098 30.3625 95
"武汉市厂前地铁站" 114.443 30.6206 94
"武汉市金银湖地铁站" 114.212 30.6452 93
"武汉市黄龙山路地铁站" 114.435 30.478 92
"武汉市长岭山地铁站" 114.57 30.4933 91
"武汉市堤角地铁站" 114.343 30.662 90
"武汉市枫林站地铁站" 114.162 30.4005 89
"武汉市中山公园地铁站" 114.278 30.5928 88
"武汉市香港路地铁站" 114.353 30.5934 87
"武汉市洪山路地铁站" 114.351 30.5517 86
"武汉市三眼桥地铁站" 114.166 30.5654 85
"武汉市红钢城地铁站" 114.419 30.6517 84
"武汉市仁和路地铁站" 114.394 30.6183 83
"武汉市三层楼地铁站" 113.977 30.6043 82
"武汉市杨汊湖地铁站" 114.267 30.6419 81
"武汉市苗栗路地铁站" 114.298 30.6032 80
"武汉市新天地铁站" 114.133 30.5584 79
"武汉市武汉站西广场站地铁站" 114.428 30.6125 78
"武汉市未来一路地铁站" 114.582 30.4501 77
"武汉市常青花园地铁站" 114.249 30.6472 76
"武汉市琴台地铁站" 114.273 30.5636 75
"武汉市虎泉地铁站" 114.377 30.5187 74
"武汉市白沙六路地铁站" 114.294 30.4754 73
"武汉市螃蟹岬地铁站" 114.325 30.5597 72
"武汉市街道口地铁站" 114.359 30.5334 71
"武汉市青鱼嘴地铁站" 114.356 30.5702 70
"武汉市马鹦路地铁站" 114.263 30.5441 69
"武汉市菱角湖路地铁站" 114.281 30.6134 68
"武汉市新荣地铁站" 114.339 30.6637 67
"武汉市工人村地铁站" 114.426 30.6558 66
"武汉市蔡甸广场地铁站" 114.047 30.5786 65
"武汉市江夏客厅地铁站" 114.328 30.395 64
"武汉市崇仁路地铁站" 114.268 30.5769 63
"武汉市军运村地铁站" 114.299 30.4315 62
"武汉市八铺街地铁站" 114.304 30.529 61
"武汉市葛店南地铁站" 114.312 30.5985 60
"武汉市光谷七路地铁站" 114.54 30.4686 59
"武汉市武汉商务区地铁站" 114.254 30.6041 58
"武汉市丹水池地铁站" 114.341 30.6492 57
"武汉市南太子湖地铁站" 114.226 30.483 56
"武汉市古田四路地铁站" 114.224 30.6121 55
"武汉市北华街地铁站" 114.337 30.3639 54
"武汉市彭刘杨路地铁站" 114.31 30.5456 53
"武汉市长港路地铁站" 114.255 30.6363 52
"武汉市未来三路地铁站" 114.613 30.4766 51
"武汉市烽火村地铁站" 114.305 30.5165 50
"武汉市额头湾地铁站" 114.162 30.6246 49
"武汉市建设二路地铁站" 114.383 30.63 48
"武汉市新农地铁站" 114.081 30.5661 47
"武汉市徐家棚地铁站" 114.349 30.587 46
"武汉市硚口路地铁站" 114.258 30.5753 45
"武汉市兴业路地铁站" 114.154 30.3362 44
"武汉市佛祖岭地铁站" 114.458 30.4639 43
"武汉市国博中心北地铁站" 114.248 30.5132 42
"武汉市光谷六路地铁站" 114.52 30.4891 41
"武汉市利济北路地铁站" 114.276 30.5802 40
"武汉市杨家湾地铁站" 114.389 30.5112 39
"武汉市协子河地铁站" 114.312 30.5985 38
"武汉市首义路地铁站" 114.315 30.535 37
"武汉市沌阳大道地铁站" 114.193 30.4852 36
"武汉市黄家湖地铁小镇地铁站" 114.305 30.4428 35
"武汉市中一路地铁站" 114.289 30.6366 34
"武汉市古田二路地铁站" 114.209 30.6176 33
"武汉市竹叶海地铁站" 114.17 30.6202 32
"武汉市光谷大道地铁站" 114.349 30.5561 31
"武汉市张家湾地铁站" 114.286 30.4886 30
"武汉市六渡桥地铁站" 114.29 30.5821 29
"武汉市汉阳客运站地铁站" 114.225 30.5601 28
"武汉市中南医院地铁站" 114.359 30.5594 27
"武汉市马影河地铁站" 114.039 30.3211 26
"武汉市积玉桥地铁站" 114.32 30.5807 25
"武汉市光谷广场地铁站" 114.403 30.5113 24
"武汉市秀湖地铁站" 114.428 30.4534 23
"武汉市新月溪公园站地铁站" 114.513 30.4853 22
"武汉市藏龙东街地铁站" 114.434 30.4444 21
"武汉市三阳路地铁站" 114.31 30.6035 20
"武汉市马池地铁站" 114.242 30.6643 19
"武汉市汉阳火车站地铁站" 114.225 30.5601 18
"武汉市金融港北地铁站" 114.427 30.4676 17
"武汉市豹澥地铁站" 114.529 30.4943 16
"武汉市园博园北地铁站" 114.222 30.6322 15
"武汉市新河街地铁站" 114.322 30.5761 14
"武汉市知音地铁站" 114.323 30.5092 13
"武汉市舵落口地铁站" 114.176 30.6155 12
"武汉市小龟山地铁站" 114.333 30.557 11
"武汉市常码头地铁站" 114.239 30.6114 10
"武汉市古田一路地铁站" 114.189 30.6055 9
"武汉市市民之家地铁站" 114.303 30.667 8
"武汉市杨园铁四院地铁站" 114.352 30.6091 7
"武汉市文昌路地铁站" 114.337 30.5035 6
"武汉市临嶂大道地铁站" 114.032 30.5728 5
"武汉市大智路地铁站" 114.301 30.5949 4
"武汉市马湖地铁站" 114.326 30.4803 3
"武汉市青龙山地铁小镇地铁站" 114.341 30.3249 2
"武汉市云飞路地铁站" 114.249 30.6087 1
parse 238
onParseFinished 110
"武汉市广埠屯地铁站" 114.371 30.5287 0
```

- 输出结果为0，m_stationInfos 空

```C++
void HttpResponseHandler::onParseFinished()
{
    qDebug() << m_stationInfos.size();
    printStationInfos(*this);
}
```

- 发现在函数 `HttpResponseHandler::onRequestFinished` 中调用了 clear，这样就导致你每次解析写入数据后下一次的 http response 到达时又把数据清空了

```C++
void HttpResponseHandler::onRequestFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // to be improved
        clear();
        QByteArray buffer = reply->readAll();
        QString context(buffer);
        // qDebug() << context;

        QString err_msg;
        if (reply->request().url().toString() == DEFAULT_WUHAN_METRO_REQUEST_URL) {       
            HttpResponseParserBase& parser = m_parserFactory.getHttpResponseParser(HttpResponseParserType::WhApp);
            parser.parse(context, err_msg);
            printSubwayLines(*this);
        }
        ...
    }

    reply->deleteLater();
}
```

- 去掉 clear 后结果正常

```
"8 : 金潭路 宏图大道 塔子湖 中一路 竹叶山 赵家条 黄浦路 徐家棚 徐东 汪家墩 岳家嘴 梨园 省博湖北日报 中南医院 水果湖 洪山路 小洪山 街道口 马房山 文治街 文昌路 省农科院 马湖 野芷湖 黄家湖地铁小镇 军运村"
"19 : 武汉站西广场站 武东站 鼓架山站 花山新城站 花山河站 光谷五路站 新月溪公园站"
"3 : 宏图大道 市民之家 后湖大道 兴业路 二七小路 罗家庄 赵家条 惠济二路 香港路 菱角湖路 范湖 云飞路 武汉商务区 双墩 宗关 王家湾 龙阳村 陶家岭 四新大道 汉阳客运站 三角湖 体育中心 东风公司 沌阳大道"
"2 : 天河机场 航空总部 宋家岗 巨龙大道 盘龙城 宏图大道 常青城 金银潭 常青花园 长港路 汉口火车站 范湖 王家墩东 青年路 中山公园 循礼门 江汉路 积玉桥 螃蟹岬 小龟山 洪山广场 中南路 宝通寺 街道口 广埠屯 虎泉 杨家湾 光谷广场 珞雄路 华中科技大学 光谷大道 佳园路 武汉东站 黄龙山路 金融港北 秀湖 藏龙东街 佛祖岭"
"1 : 汉口北 滠口新城 滕子岗 堤角 新荣 丹水池 徐州新村 二七路 头道街 黄浦路 三阳路 大智路 循礼门 友谊路 利济北路 崇仁路 硚口路 太平洋 宗关 汉西一路 古田四路 古田三路 古田二路 古田一路 舵落口 竹叶海 额头湾 五环大道 东吴大道 码头潭公园 三店 径河"
"16 : 国博中心南 老关村 南太子湖 沌口 小军山 枫林站 大军山 桂子湖 马影河 协子河 湾湖 周家河 纱帽 通航机场"
"7 : 横店 裕福路 天阳大道 腾龙大道 巨龙大道 汤云海路 马池 园博园北 园博园 常码头 武汉商务区 王家墩东 取水楼 香港路 三阳路 徐家棚 湖北大学 新河街 螃蟹岬 小东门 武昌火车站 瑞安街 建安街 湖工大 板桥 野芷湖 新路村 大花岭 江夏客厅 谭鑫培公园 北华街 纸坊大街 青龙山地铁小镇"
"6 : 新城十一路 码头潭公园 五环体育中心 二雅路 海口三路 金银湖公园 金银湖 园博园北 轻工大学 常青花园 杨汊湖 石桥 唐家墩 三眼桥 香港路 苗栗路 大智路 江汉路 六渡桥 汉正街 武胜路 琴台 钟家村 马鹦路 建港 前进村 国博中心北 国博中心南 老关村 江城大道 车城东路 东风公司"
"5 : 红霞 黄家湖（武科大） 中医药大学 白沙六路 光霞 张家湾 烽火村 八铺街 复兴路 彭刘杨路 司门口黄鹤楼 昙华林武胜门 积玉桥 三层楼 三角路 徐家棚 杨园铁四院 余家头 科普公园 建设二路 和平公园 红钢城 青宜居 工人村 武钢 厂前 武汉站东广场"
"4 : 武汉火车站 杨春湖 工业四路 仁和路 园林路 罗家港 铁机路 岳家嘴 东亭 青鱼嘴 楚河汉街 洪山广场 中南路 梅苑小区 武昌火车站 首义路 复兴路 拦江路 钟家村 汉阳火车站 五里墩 七里庙 十里铺 王家湾 玉龙路 永安堂 孟家铺 黄金口 新天 集贤 知音 新农 凤凰路 蔡甸广场 临嶂大道 新庙村 柏林"
"11 : 武汉东站 湖口 光谷同济医院 光谷生物园 光谷四路 光谷五路 光谷六路 豹澥 光谷七路 长岭山 未来一路 未来三路 左岭 葛店南"
"武汉市徐家棚地铁站" 114.349 30.587 248
"武汉市堤角地铁站" 114.343 30.662 247
"武汉市常青城地铁站" 114.261 30.6667 246
"武汉市二七路地铁站" 114.319 30.6335 245
"武汉市常青花园地铁站" 114.249 30.6472 244
"武汉市汉阳客运站地铁站" 114.225 30.5601 243
"武汉市五环大道地铁站" 114.145 30.6193 242
"武汉市云飞路地铁站" 114.249 30.6087 241
"武汉市岳家嘴地铁站" 114.369 30.5834 240
"武汉市黄金口地铁站" 114.044 30.4789 239
"武汉市友谊路地铁站" 114.407 30.625 238
"武汉市江夏客厅地铁站" 114.328 30.395 237
"武汉市凤凰路地铁站" 114.315 30.4935 236
"武汉市工人村地铁站" 114.426 30.6558 235
"武汉市湾湖地铁站" 113.839 30.3853 234
"武汉市三角湖地铁站" 114.186 30.5222 233
"武汉市光霞地铁站" 114.3 30.4836 232
"武汉市玉龙路地铁站" 114.21 30.5773 231
"武汉市舵落口地铁站" 114.176 30.6155 230
"武汉市新河街地铁站" 114.322 30.5761 229
"武汉市江城大道地铁站" 114.23 30.5304 228
"武汉市国博中心南地铁站" 114.248 30.5132 227
"武汉市汉阳火车站地铁站" 114.225 30.5601 226
"武汉市滕子岗地铁站" 114.312 30.5985 225
"武汉市黄家湖（武科大）地铁站" 114.347 30.5218 224
"武汉市大花岭地铁站" 114.319 30.417 223
"武汉市大智路地铁站" 114.301 30.5949 222
"武汉市古田四路地铁站" 114.224 30.6121 221
"武汉市罗家港地铁站" 114.378 30.5992 220
"武汉市裕福路地铁站" 114.312 30.5985 219
"武汉市园博园北地铁站" 114.222 30.6322 218
"武汉市常码头地铁站" 114.239 30.6114 217
"武汉市中南路地铁站" 114.34 30.5454 216
"武汉市国博中心北地铁站" 114.248 30.5132 215
"武汉市华中科技大学地铁站" 114.267 30.5887 214
"武汉市长岭山地铁站" 114.57 30.4933 213
"武汉市老关村地铁站" 114.225 30.5189 212
"武汉市双墩地铁站" 114.239 30.593 211
"武汉市园博园地铁站" 114.222 30.6322 210
"武汉市张家湾地铁站" 114.286 30.4886 209
"武汉市东风公司地铁站" 114.172 30.5087 208
"武汉市豹澥地铁站" 114.529 30.4943 207
"武汉市青鱼嘴地铁站" 114.356 30.5702 206
"武汉市头道街地铁站" 114.324 30.6239 205
"武汉市市民之家地铁站" 114.303 30.667 204
"武汉市汪家墩地铁站" 114.357 30.5915 203
"武汉市中医药大学地铁站" 114.277 30.4538 202
"武汉市楚河汉街地铁站" 114.345 30.5658 201
"武汉市宗关地铁站" 114.236 30.5851 200
"武汉市通航机场地铁站" 114.312 30.5985 199
"武汉市径河地铁站" 114.127 30.6654 198
"武汉市新路村地铁站" 114.321 30.4508 197
"武汉市码头潭公园地铁站" 114.133 30.6422 196
"武汉市蔡甸广场地铁站" 114.047 30.5786 195
"武汉市汉口北地铁站" 114.322 30.7054 194
"武汉市太平洋地铁站" 114.247 30.5804 193
"武汉市文昌路地铁站" 114.337 30.5035 192
"武汉市腾龙大道地铁站" 114.283 30.727 191
"武汉市体育中心地铁站" 114.174 30.6896 190
"武汉市武汉火车站地铁站" 114.312 30.5985 189
"武汉市园林路地铁站" 114.243 30.9414 188
"武汉市天阳大道地铁站" 114.312 30.5985 187
"武汉市瑞安街地铁站" 114.325 30.5186 186
"武汉市螃蟹岬地铁站" 114.325 30.5597 185
"武汉市汉口火车站地铁站" 114.262 30.6216 184
"武汉市光谷七路地铁站" 114.54 30.4686 183
"武汉市三店地铁站" 114.312 30.5985 182
"武汉市光谷大道地铁站" 114.349 30.5561 181
"武汉市武钢地铁站" 114.471 30.6015 180
"武汉市昙华林武胜门地铁站" 114.312 30.5596 179
"武汉市利济北路地铁站" 114.276 30.5802 178
"武汉市青宜居地铁站" 114.427 30.6496 177
"武汉市新农地铁站" 114.081 30.5661 176
"武汉市罗家庄地铁站" 114.31 30.6291 175
"武汉市湖北大学地铁站" 114.336 30.5837 174
"武汉市湖工大地铁站" 114.32 30.4896 173
"武汉市金银潭地铁站" 114.271 30.6661 172
"武汉市纱帽地铁站" 114.092 30.32 171
"武汉市唐家墩地铁站" 114.278 30.6236 170
"武汉市马影河地铁站" 114.039 30.3211 169
"武汉市金银湖公园地铁站" 114.176 30.6604 168
"武汉市六渡桥地铁站" 114.29 30.5821 167
"武汉市三阳路地铁站" 114.31 30.6035 166
"武汉市花山河站地铁站" 114.519 30.5471 165
"武汉市五里墩地铁站" 114.247 30.5602 164
"武汉市虎泉地铁站" 114.377 30.5187 163
"武汉市杨汊湖地铁站" 114.267 30.6419 162
"武汉市石桥地铁站" 114.329 30.3401 161
"武汉市长港路地铁站" 114.255 30.6363 160
"武汉市秀湖地铁站" 114.428 30.4534 159
"武汉市彭刘杨路地铁站" 114.31 30.5456 158
"武汉市竹叶山地铁站" 114.292 30.6275 157
"武汉市江汉路地铁站" 114.292 30.5913 156
"武汉市龙阳村地铁站" 114.211 30.5599 155
"武汉市马房山地铁站" 114.356 30.521 154
"武汉市光谷广场地铁站" 114.403 30.5113 153
"武汉市积玉桥地铁站" 114.32 30.5807 152
"武汉市鼓架山站地铁站" 114.483 30.565 151
"武汉市花山新城站地铁站" 114.496 30.5656 150
"武汉市知音地铁站" 114.323 30.5092 149
"武汉市徐东地铁站" 114.354 30.5943 148
"武汉市红钢城地铁站" 114.419 30.6517 147
"武汉市省博湖北日报地铁站" 114.369 30.5714 146
"武汉市循礼门地铁站" 114.29 30.5918 145
"武汉市车城东路地铁站" 114.184 30.4902 144
"武汉市菱角湖路地铁站" 114.281 30.6134 143
"武汉市武汉站西广场站地铁站" 114.428 30.6125 142
"武汉市王家墩东地铁站" 114.312 30.5985 141
"武汉市余家头地铁站" 114.361 30.6154 140
"武汉市纸坊大街地铁站" 114.306 30.3465 139
"武汉市首义路地铁站" 114.315 30.535 138
"武汉市横店地铁站" 114.306 30.8088 137
"武汉市武东站地铁站" 114.437 30.4921 136
"武汉市塔子湖地铁站" 114.287 30.6585 135
"武汉市建设二路地铁站" 114.383 30.63 134
"武汉市汉西一路地铁站" 114.227 30.5922 133
"武汉市取水楼地铁站" 114.278 30.598 132
"武汉市崇仁路地铁站" 114.268 30.5769 131
"武汉市钟家村地铁站" 114.273 30.5553 130
"武汉市光谷六路地铁站" 114.52 30.4891 129
"武汉市马湖地铁站" 114.326 30.4803 128
"武汉市武汉东站地铁站" 114.437 30.4921 127
"武汉市武汉商务区地铁站" 114.254 30.6041 126
"武汉市三角路地铁站" 114.331 30.5844 125
"武汉市古田二路地铁站" 114.209 30.6176 124
"武汉市沌口地铁站" 114.158 30.4957 123
"武汉市香港路地铁站" 114.353 30.5934 122
"武汉市厂前地铁站" 114.443 30.6206 121
"武汉市光谷五路地铁站" 114.509 30.4595 120
"武汉市藏龙东街地铁站" 114.434 30.4444 119
"武汉市集贤地铁站" 131.255 46.8217 118
"武汉市光谷生物园地铁站" 114.548 30.4843 117
"武汉市南太子湖地铁站" 114.226 30.483 116
"武汉市东吴大道地铁站" 114.162 30.629 115
"武汉市梨园地铁站" 114.376 30.581 114
"武汉市盘龙城地铁站" 114.266 30.7064 113
"武汉市广埠屯地铁站" 114.371 30.5287 112
"武汉市洪山路地铁站" 114.351 30.5517 111
"武汉市杨春湖地铁站" 114.423 30.611 110
"武汉市珞雄路地铁站" 114.413 30.5081 109
"武汉市宋家岗地铁站" 114.239 30.7316 108
"武汉市佛祖岭地铁站" 114.458 30.4639 107
"武汉市二七小路地铁站" 114.312 30.5985 106
"武汉市汉正街地铁站" 114.275 30.5708 105
"武汉市未来三路地铁站" 114.613 30.4766 104
"武汉市轻工大学地铁站" 114.241 30.6459 103
"武汉市拦江路地铁站" 114.276 30.5487 102
"武汉市小龟山地铁站" 114.333 30.557 101
"武汉市陶家岭地铁站" 114.208 30.5514 100
"武汉市谭鑫培公园地铁站" 114.33 30.3754 99
"武汉市洪山广场地铁站" 114.343 30.5514 98
"武汉市杨家湾地铁站" 114.389 30.5112 97
"武汉市马池地铁站" 114.242 30.6643 96
"武汉市小军山地铁站" 114.176 30.4241 95
"武汉市街道口地铁站" 114.359 30.5334 94
"武汉市协子河地铁站" 114.312 30.5985 93
"武汉市丹水池地铁站" 114.341 30.6492 92
"武汉市黄浦路地铁站" 114.307 30.6331 91
"武汉市黄家湖地铁小镇地铁站" 114.305 30.4428 90
"武汉市马鹦路地铁站" 114.263 30.5441 89
"武汉市科普公园地铁站" 114.37 30.6233 88
"武汉市枫林站地铁站" 114.162 30.4005 87
"武汉市古田三路地铁站" 114.211 30.6015 86
"武汉市新天地铁站" 114.133 30.5584 85
"武汉市左岭地铁站" 114.617 30.531 84
"武汉市滠口新城地铁站" 114.349 30.6903 83
"武汉市建港地铁站" 114.261 30.5274 82
"武汉市五环体育中心地铁站" 114.15 30.6464 81
"武汉市宏图大道地铁站" 114.286 30.6718 80
"武汉市黄龙山路地铁站" 114.435 30.478 79
"武汉市惠济二路地铁站" 114.296 30.6199 78
"武汉市军运村地铁站" 114.299 30.4315 77
"武汉市苗栗路地铁站" 114.298 30.6032 76
"武汉市青年路地铁站" 114.273 30.5874 75
"武汉市三眼桥地铁站" 114.166 30.5654 74
"武汉市小东门地铁站" 114.323 30.5496 73
"武汉市东亭地铁站" 114.366 30.5728 72
"武汉市新月溪公园站地铁站" 114.513 30.4853 71
"武汉市青龙山地铁小镇地铁站" 114.341 30.3249 70
"武汉市小洪山地铁站" 114.356 30.5453 69
"武汉市孟家铺地铁站" 114.312 30.5985 68
"武汉市新庙村地铁站" 114.019 30.5903 67
"武汉市赵家条地铁站" 114.304 30.6237 66
"武汉市天河机场地铁站" 114.236 30.8077 65
"武汉市兴业路地铁站" 114.154 30.3362 64
"武汉市四新大道地铁站" 114.208 30.534 63
"武汉市白沙六路地铁站" 114.294 30.4754 62
"武汉市桂子湖地铁站" 114.098 30.3625 61
"武汉市中山公园地铁站" 114.278 30.5928 60
"武汉市八铺街地铁站" 114.304 30.529 59
"武汉市金银湖地铁站" 114.212 30.6452 58
"武汉市新荣地铁站" 114.339 30.6637 57
"武汉市光谷四路地铁站" 114.495 30.4598 56
"武汉市建安街地铁站" 114.309 30.5113 55
"武汉市武胜路地铁站" 114.275 30.5767 54
"武汉市武汉站东广场地铁站" 114.433 30.6136 53
"武汉市硚口路地铁站" 114.258 30.5753 52
"武汉市武昌火车站地铁站" 114.325 30.5427 51
"武汉市北华街地铁站" 114.337 30.3639 50
"武汉市新城十一路地铁站" 114.118 30.6333 49
"武汉市工业四路地铁站" 114.314 30.4845 48
"武汉市梅苑小区地铁站" 114.334 30.5379 47
"武汉市野芷湖地铁站" 114.348 30.4717 46
"武汉市海口三路地铁站" 114.172 30.6455 45
"武汉市宝通寺地铁站" 114.347 30.538 44
"武汉市三层楼地铁站" 113.977 30.6043 43
"武汉市汤云海路地铁站" 114.255 30.7058 42
"武汉市琴台地铁站" 114.273 30.5636 41
"武汉市司门口黄鹤楼地铁站" 114.307 30.5527 40
"武汉市光谷同济医院地铁站" 114.472 30.4944 39
"武汉市葛店南地铁站" 114.312 30.5985 38
"武汉市佳园路地铁站" 114.433 30.4921 37
"武汉市杨园铁四院地铁站" 114.352 30.6091 36
"武汉市红霞地铁站" 114.268 30.4366 35
"武汉市永安堂地铁站" 114.194 30.5711 34
"武汉市柏林地铁站" 113.992 30.5893 33
"武汉市十里铺地铁站" 114.312 30.5985 32
"武汉市沌阳大道地铁站" 114.193 30.4852 31
"武汉市复兴路地铁站" 114.307 30.5304 30
"武汉市中一路地铁站" 114.289 30.6366 29
"武汉市周家河地铁站" 114.874 30.8425 28
"武汉市临嶂大道地铁站" 114.032 30.5728 27
"武汉市板桥地铁站" 114.32 30.48 26
"武汉市范湖地铁站" 114.234 30.6198 25
"武汉市省农科院地铁站" 114.328 30.4903 24
"武汉市后湖大道地铁站" 114.325 30.6614 23
"武汉市铁机路地铁站" 114.356 30.6127 22
"武汉市光谷五路站地铁站" 114.509 30.4595 21
"武汉市湖口地铁站" 116.258 29.7371 20
"武汉市竹叶海地铁站" 114.17 30.6202 19
"武汉市古田一路地铁站" 114.189 30.6055 18
"武汉市金潭路地铁站" 114.286 30.6834 17
"武汉市额头湾地铁站" 114.162 30.6246 16
"武汉市王家湾地铁站" 114.218 30.5714 15
"武汉市水果湖地铁站" 114.353 30.5585 14
"武汉市前进村地铁站" 114.263 30.5295 13
"武汉市和平公园地铁站" 114.392 30.6396 12
"武汉市文治街地铁站" 114.341 30.508 11
"武汉市二雅路地铁站" 114.157 30.6313 10
"武汉市徐州新村地铁站" 114.329 30.6373 9
"武汉市巨龙大道地铁站" 114.295 30.7085 8
"武汉市大军山地铁站" 114.091 30.4116 7
"武汉市烽火村地铁站" 114.305 30.5165 6
"武汉市仁和路地铁站" 114.394 30.6183 5
"武汉市七里庙地铁站" 114.238 30.5627 4
"武汉市航空总部地铁站" 114.234 30.7392 3
"武汉市金融港北地铁站" 114.427 30.4676 2
"武汉市中南医院地铁站" 114.359 30.5594 1
249
"大花岭 2 114.319 30.417 7"
"和平公园 2 114.392 30.6396 5"
"余家头 2 114.361 30.6154 5"
"未来三路 2 114.613 30.4766 11"
"未来一路 2 114.582 30.4501 11"
"黄龙山路 2 114.435 30.478 2"
"大军山 2 114.091 30.4116 16"
"常青城 2 114.261 30.6667 2"
"常码头 2 114.239 30.6114 7"
"金银湖 2 114.212 30.6452 6"
"中一路 2 114.289 30.6366 8"
"螃蟹岬 2 114.325 30.5597 2 7"
"额头湾 2 114.162 30.6246 1"
"岳家嘴 2 114.369 30.5834 8 4"
"三角路 2 114.331 30.5844 5"
"周家河 2 114.874 30.8425 16"
"光谷五路站 2 114.509 30.4595 19"
"罗家港 2 114.378 30.5992 4"
"虎泉 2 114.377 30.5187 2"
"苗栗路 2 114.298 30.6032 6"
"老关村 2 114.225 30.5189 16 6"
"陶家岭 2 114.208 30.5514 3"
"八铺街 2 114.304 30.529 5"
"三层楼 2 113.977 30.6043 5"
"宋家岗 2 114.239 30.7316 2"
"新月溪公园站 2 114.513 30.4853 19"
"光谷大道 2 114.349 30.5561 2"
"小东门 2 114.323 30.5496 7"
"洪山路 2 114.351 30.5517 8"
"国博中心北 2 114.248 30.5132 6"
"汉西一路 2 114.227 30.5922 1"
"新农 2 114.081 30.5661 4"
"武昌火车站 2 114.325 30.5427 7 4"
"常青花园 2 114.249 30.6472 2 6"
"头道街 2 114.324 30.6239 1"
"新城十一路 2 114.118 30.6333 6"
"洪山广场 2 114.343 30.5514 2 4"
"建设二路 2 114.383 30.63 5"
"利济北路 2 114.276 30.5802 1"
"金融港北 2 114.427 30.4676 2"
"工业四路 2 114.314 30.4845 4"
"赵家条 2 114.304 30.6237 8 3"
"硚口路 2 114.258 30.5753 1"
"南太子湖 2 114.226 30.483 16"
"纸坊大街 2 114.306 30.3465 7"
"惠济二路 2 114.296 30.6199 3"
"蔡甸广场 2 114.047 30.5786 4"
"梅苑小区 2 114.334 30.5379 4"
"五环大道 2 114.145 30.6193 1"
"文昌路 2 114.337 30.5035 8"
"长港路 2 114.255 30.6363 2"
"武汉火车站 2 114.312 30.5985 4"
"古田二路 2 114.209 30.6176 1"
"秀湖 2 114.428 30.4534 2"
"友谊路 2 114.407 30.625 1"
"裕福路 2 114.312 30.5985 7"
"二七路 2 114.319 30.6335 1"
"杨家湾 2 114.389 30.5112 2"
"首义路 2 114.315 30.535 4"
"桂子湖 2 114.098 30.3625 16"
"光谷广场 2 114.403 30.5113 2"
"铁机路 2 114.356 30.6127 4"
"龙阳村 2 114.211 30.5599 3"
"市民之家 2 114.303 30.667 3"
"柏林 2 113.992 30.5893 4"
"马房山 2 114.356 30.521 8"
"昙华林武胜门 2 114.312 30.5596 5"
"武胜路 2 114.275 30.5767 6"
"武汉站西广场站 2 114.428 30.6125 19"
"三店 2 114.312 30.5985 1"
"湾湖 2 113.839 30.3853 16"
"华中科技大学 2 114.267 30.5887 2"
"光谷五路 2 114.509 30.4595 11"
"葛店南 2 114.312 30.5985 11"
"码头潭公园 2 114.133 30.6422 1 6"
"青年路 2 114.273 30.5874 2"
"光谷同济医院 2 114.472 30.4944 11"
"崇仁路 2 114.268 30.5769 1"
"二雅路 2 114.157 30.6313 6"
"杨汊湖 2 114.267 30.6419 6"
"佛祖岭 2 114.458 30.4639 2"
"光谷生物园 2 114.548 30.4843 11"
"积玉桥 2 114.32 30.5807 2 5"
"中山公园 2 114.278 30.5928 2"
"二七小路 2 114.312 30.5985 3"
"张家湾 2 114.286 30.4886 5"
"仁和路 2 114.394 30.6183 4"
"钟家村 2 114.273 30.5553 6 4"
"东亭 2 114.366 30.5728 4"
"光谷七路 2 114.54 30.4686 11"
"广埠屯 2 114.371 30.5287 2"
"马影河 2 114.039 30.3211 16"
"三角湖 2 114.186 30.5222 3"
"四新大道 2 114.208 30.534 3"
"十里铺 2 114.312 30.5985 4"
"科普公园 2 114.37 30.6233 5"
"江汉路 2 114.292 30.5913 2 6"
"豹澥 2 114.529 30.4943 11"
"径河 2 114.127 30.6654 1"
"宏图大道 2 114.286 30.6718 8 3 2"
"汉阳火车站 2 114.225 30.5601 4"
"宗关 2 114.236 30.5851 3 1"
"腾龙大道 2 114.283 30.727 7"
"堤角 2 114.343 30.662 1"
"五环体育中心 2 114.15 30.6464 6"
"光谷六路 2 114.52 30.4891 11"
"王家湾 2 114.218 30.5714 3 4"
"云飞路 2 114.249 30.6087 3"
"滠口新城 2 114.349 30.6903 1"
"纱帽 2 114.092 30.32 16"
"光谷四路 2 114.495 30.4598 11"
"凤凰路 2 114.315 30.4935 4"
"枫林站 2 114.162 30.4005 16"
"杨园铁四院 2 114.352 30.6091 5"
"临嶂大道 2 114.032 30.5728 4"
"军运村 2 114.299 30.4315 8"
"航空总部 2 114.234 30.7392 2"
"中医药大学 2 114.277 30.4538 5"
"汉口火车站 2 114.262 30.6216 2"
"古田三路 2 114.211 30.6015 1"
"武东站 2 114.437 30.4921 19"
"后湖大道 2 114.325 30.6614 3"
"中南医院 2 114.359 30.5594 8"
"建安街 2 114.309 30.5113 7"
"车城东路 2 114.184 30.4902 6"
"复兴路 2 114.307 30.5304 5 4"
"范湖 2 114.234 30.6198 3 2"
"建港 2 114.261 30.5274 6"
"通航机场 2 114.312 30.5985 16"
"彭刘杨路 2 114.31 30.5456 5"
"知音 2 114.323 30.5092 4"
"街道口 2 114.359 30.5334 8 2"
"鼓架山站 2 114.483 30.565 19"
"汪家墩 2 114.357 30.5915 8"
"徐州新村 2 114.329 30.6373 1"
"光霞 2 114.3 30.4836 5"
"巨龙大道 2 114.295 30.7085 2 7"
"六渡桥 2 114.29 30.5821 6"
"武钢 2 114.471 30.6015 5"
"楚河汉街 2 114.345 30.5658 4"
"塔子湖 2 114.287 30.6585 8"
"司门口黄鹤楼 2 114.307 30.5527 5"
"宝通寺 2 114.347 30.538 2"
"海口三路 2 114.172 30.6455 6"
"天河机场 2 114.236 30.8077 2"
"杨春湖 2 114.423 30.611 4"
"王家墩东 2 114.312 30.5985 2 7"
"唐家墩 2 114.278 30.6236 6"
"金潭路 2 114.286 30.6834 8"
"湖北大学 2 114.336 30.5837 7"
"瑞安街 2 114.325 30.5186 7"
"竹叶山 2 114.292 30.6275 8"
"盘龙城 2 114.266 30.7064 2"
"拦江路 2 114.276 30.5487 4"
"藏龙东街 2 114.434 30.4444 2"
"大智路 2 114.301 30.5949 1 6"
"新路村 2 114.321 30.4508 7"
"小龟山 2 114.333 30.557 2"
"马湖 2 114.326 30.4803 8"
"三眼桥 2 114.166 30.5654 6"
"左岭 2 114.617 30.531 11"
"小洪山 2 114.356 30.5453 8"
"长岭山 2 114.57 30.4933 11"
"三阳路 2 114.31 30.6035 1 7"
"罗家庄 2 114.31 30.6291 3"
"国博中心南 2 114.248 30.5132 16 6"
"梨园 2 114.376 30.581 8"
"孟家铺 2 114.312 30.5985 4"
"古田四路 2 114.224 30.6121 1"
"滕子岗 2 114.312 30.5985 1"
"前进村 2 114.263 30.5295 6"
"厂前 2 114.443 30.6206 5"
"黄家湖（武科大） 2 114.347 30.5218 5"
"协子河 2 114.312 30.5985 16"
"烽火村 2 114.305 30.5165 5"
"省农科院 2 114.328 30.4903 8"
"竹叶海 2 114.17 30.6202 1"
"新河街 2 114.322 30.5761 7"
"马鹦路 2 114.263 30.5441 6"
"琴台 2 114.273 30.5636 6"
"青鱼嘴 2 114.356 30.5702 4"
"新天 2 114.133 30.5584 4"
"集贤 2 131.255 46.8217 4"
"武汉东站 2 114.437 30.4921 2 11"
"新荣 2 114.339 30.6637 1"
"金银湖公园 2 114.176 30.6604 6"
"汉正街 2 114.275 30.5708 6"
"香港路 2 114.353 30.5934 3 7 6"
"五里墩 2 114.247 30.5602 4"
"双墩 2 114.239 30.593 3"
"玉龙路 2 114.21 30.5773 4"
"汤云海路 2 114.255 30.7058 7"
"北华街 2 114.337 30.3639 7"
"体育中心 2 114.174 30.6896 3"
"徐家棚 2 114.349 30.587 8 7 5"
"江夏客厅 2 114.328 30.395 7"
"花山河站 2 114.519 30.5471 19"
"东吴大道 2 114.162 30.629 1"
"野芷湖 2 114.348 30.4717 8 7"
"太平洋 2 114.247 30.5804 1"
"花山新城站 2 114.496 30.5656 19"
"黄家湖地铁小镇 2 114.305 30.4428 8"
"园博园北 2 114.222 30.6322 7 6"
"黄浦路 2 114.307 30.6331 8 1"
"板桥 2 114.32 30.48 7"
"金银潭 2 114.271 30.6661 2"
"沌阳大道 2 114.193 30.4852 3"
"兴业路 2 114.154 30.3362 3"
"中南路 2 114.34 30.5454 2 4"
"水果湖 2 114.353 30.5585 8"
"徐东 2 114.354 30.5943 8"
"东风公司 2 114.172 30.5087 3 6"
"小军山 2 114.176 30.4241 16"
"文治街 2 114.341 30.508 8"
"省博湖北日报 2 114.369 30.5714 8"
"汉口北 2 114.322 30.7054 1"
"青龙山地铁小镇 2 114.341 30.3249 7"
"白沙六路 2 114.294 30.4754 5"
"黄金口 2 114.044 30.4789 4"
"石桥 2 114.329 30.3401 6"
"古田一路 2 114.189 30.6055 1"
"武汉站东广场 2 114.433 30.6136 5"
"舵落口 2 114.176 30.6155 1"
"永安堂 2 114.194 30.5711 4"
"沌口 2 114.158 30.4957 16"
"青宜居 2 114.427 30.6496 5"
"循礼门 2 114.29 30.5918 2 1"
"轻工大学 2 114.241 30.6459 6"
"园博园 2 114.222 30.6322 7"
"横店 2 114.306 30.8088 7"
"湖口 2 116.258 29.7371 11"
"园林路 2 114.243 30.9414 4"
"新庙村 2 114.019 30.5903 4"
"菱角湖路 2 114.281 30.6134 3"
"谭鑫培公园 2 114.33 30.3754 7"
"马池 2 114.242 30.6643 7"
"江城大道 2 114.23 30.5304 6"
"七里庙 2 114.238 30.5627 4"
"红霞 2 114.268 30.4366 5"
"丹水池 2 114.341 30.6492 1"
"红钢城 2 114.419 30.6517 5"
"工人村 2 114.426 30.6558 5"
"天阳大道 2 114.312 30.5985 7"
"汉阳客运站 2 114.225 30.5601 3"
"武汉商务区 2 114.254 30.6041 3 7"
"佳园路 2 114.433 30.4921 2"
"珞雄路 2 114.413 30.5081 2"
"湖工大 2 114.32 30.4896 7"
"取水楼 2 114.278 30.598 7"
"武汉市未来一路地铁站" 114.582 30.4501 0
```



# 网络模块自测结果

```
"8 : 金潭路 宏图大道 塔子湖 中一路 竹叶山 赵家条 黄浦路 徐家棚 徐东 汪家墩 岳家嘴 梨园 省博湖北日报 中南医院 水果湖 洪山路 小洪山 街道口 马房山 文治街 文昌路 省农科院 马湖 野芷湖 黄家湖地铁小镇 军运村"
"11 : 武汉东站 湖口 光谷同济医院 光谷生物园 光谷四路 光谷五路 光谷六路 豹澥 光谷七路 长岭山 未来一路 未来三路 左岭 葛店南"
"4 : 武汉火车站 杨春湖 工业四路 仁和路 园林路 罗家港 铁机路 岳家嘴 东亭 青鱼嘴 楚河汉街 洪山广场 中南路 梅苑小区 武昌火车站 首义路 复兴路 拦江路 钟家村 汉阳火车站 五里墩 七里庙 十里铺 王家湾 玉龙路 永安堂 孟家铺 黄金口 新天 集贤 知音 新农 凤凰路 蔡甸广场 临嶂大道 新庙村 柏林"
"5 : 红霞 黄家湖（武科大） 中医药大学 白沙六路 光霞 张家湾 烽火村 八铺街 复兴路 彭刘杨路 司门口黄鹤楼 昙华林武胜门 积玉桥 三层楼 三角路 徐家棚 杨园铁四院 余家头 科普公园 建设二路 和平公园 红钢城 青宜居 工人村 武钢 厂前 武汉站东广场"
"6 : 新城十一路 码头潭公园 五环体育中心 二雅路 海口三路 金银湖公园 金银湖 园博园北 轻工大学 常青花园 杨汊湖 石桥 唐家墩 三眼桥 香港路 苗栗路 大智路 江汉路 六渡桥 汉正街 武胜路 琴台 钟家村 马鹦路 建港 前进村 国博中心北 国博中心南 老关村 江城大道 车城东路 东风公司"
"7 : 横店 裕福路 天阳大道 腾龙大道 巨龙大道 汤云海路 马池 园博园北 园博园 常码头 武汉商务区 王家墩东 取水楼 香港路 三阳路 徐家棚 湖北大学 新河街 螃蟹岬 小东门 武昌火车站 瑞安街 建安街 湖工大 板桥 野芷湖 新路村 大花岭 江夏客厅 谭鑫培公园 北华街 纸坊大街 青龙山地铁小镇"
"1 : 汉口北 滠口新城 滕子岗 堤角 新荣 丹水池 徐州新村 二七路 头道街 黄浦路 三阳路 大智路 循礼门 友谊路 利济北路 崇仁路 硚口路 太平洋 宗关 汉西一路 古田四路 古田三路 古田二路 古田一路 舵落口 竹叶海 额头湾 五环大道 东吴大道 码头潭公园 三店 径河"
"16 : 国博中心南 老关村 南太子湖 沌口 小军山 枫林站 大军山 桂子湖 马影河 协子河 湾湖 周家河 纱帽 通航机场"
"2 : 天河机场 航空总部 宋家岗 巨龙大道 盘龙城 宏图大道 常青城 金银潭 常青花园 长港路 汉口火车站 范湖 王家墩东 青年路 中山公园 循礼门 江汉路 积玉桥 螃蟹岬 小龟山 洪山广场 中南路 宝通寺 街道口 广埠屯 虎泉 杨家湾 光谷广场 珞雄路 华中科技大学 光谷大道 佳园路 武汉东站 黄龙山路 金融港北 秀湖 藏龙东街 佛祖岭"
"3 : 宏图大道 市民之家 后湖大道 兴业路 二七小路 罗家庄 赵家条 惠济二路 香港路 菱角湖路 范湖 云飞路 武汉商务区 双墩 宗关 王家湾 龙阳村 陶家岭 四新大道 汉阳客运站 三角湖 体育中心 东风公司 沌阳大道"
"19 : 武汉站西广场站 武东站 鼓架山站 花山新城站 花山河站 光谷五路站 新月溪公园站"
"武汉市长岭山地铁站" 114.57 30.4933 248
"武汉市杨汊湖地铁站" 114.267 30.6419 247
"武汉市裕福路地铁站" 114.312 30.5985 246
"武汉市武汉火车站地铁站" 114.312 30.5985 245
"武汉市黄家湖地铁小镇地铁站" 114.305 30.4428 244
"武汉市鼓架山站地铁站" 114.483 30.565 243
"武汉市五环大道地铁站" 114.145 30.6193 242
"武汉市杨园铁四院地铁站" 114.352 30.6091 241
"武汉市丹水池地铁站" 114.341 30.6492 240
"武汉市新城十一路地铁站" 114.118 30.6333 239
"武汉市科普公园地铁站" 114.37 30.6233 238
"武汉市文昌路地铁站" 114.337 30.5035 237
"武汉市三眼桥地铁站" 114.166 30.5654 236
"武汉市径河地铁站" 114.127 30.6654 235
"武汉市藏龙东街地铁站" 114.434 30.4444 234
"武汉市老关村地铁站" 114.225 30.5189 233
"武汉市金银潭地铁站" 114.271 30.6661 232
"武汉市拦江路地铁站" 114.276 30.5487 231
"武汉市未来三路地铁站" 114.613 30.4766 230
"武汉市古田三路地铁站" 114.211 30.6015 229
"武汉市天河机场地铁站" 114.236 30.8077 228
"武汉市徐家棚地铁站" 114.349 30.587 227
"武汉市赵家条地铁站" 114.304 30.6237 226
"武汉市杨春湖地铁站" 114.423 30.611 225
"武汉市新庙村地铁站" 114.019 30.5903 224
"武汉市新路村地铁站" 114.321 30.4508 223
"武汉市省博湖北日报地铁站" 114.369 30.5714 222
"武汉市宗关地铁站" 114.236 30.5851 221
"武汉市常青城地铁站" 114.261 30.6667 220
"武汉市范湖地铁站" 114.234 30.6198 219
"武汉市厂前地铁站" 114.443 30.6206 218
"武汉市利济北路地铁站" 114.276 30.5802 217
"武汉市光谷五路站地铁站" 114.509 30.4595 216
"武汉市马房山地铁站" 114.356 30.521 215
"武汉市黄家湖（武科大）地铁站" 114.347 30.5218 214
"武汉市红钢城地铁站" 114.419 30.6517 213
"武汉市中南医院地铁站" 114.359 30.5594 212
"武汉市金银湖地铁站" 114.212 30.6452 211
"武汉市湖北大学地铁站" 114.336 30.5837 210
"武汉市中一路地铁站" 114.289 30.6366 209
"武汉市工业四路地铁站" 114.314 30.4845 208
"武汉市码头潭公园地铁站" 114.133 30.6422 207
"武汉市纸坊大街地铁站" 114.306 30.3465 206
"武汉市左岭地铁站" 114.617 30.531 205
"武汉市循礼门地铁站" 114.29 30.5918 204
"武汉市珞雄路地铁站" 114.413 30.5081 203
"武汉市小洪山地铁站" 114.356 30.5453 202
"武汉市中南路地铁站" 114.34 30.5454 201
"武汉市舵落口地铁站" 114.176 30.6155 200
"武汉市通航机场地铁站" 114.312 30.5985 199
"武汉市湾湖地铁站" 113.839 30.3853 198
"武汉市省农科院地铁站" 114.328 30.4903 197
"武汉市东吴大道地铁站" 114.162 30.629 196
"武汉市纱帽地铁站" 114.092 30.32 195
"武汉市龙阳村地铁站" 114.211 30.5599 194
"武汉市豹澥地铁站" 114.529 30.4943 193
"武汉市马影河地铁站" 114.039 30.3211 192
"武汉市佳园路地铁站" 114.433 30.4921 191
"武汉市园林路地铁站" 114.243 30.9414 190
"武汉市广埠屯地铁站" 114.371 30.5287 189
"武汉市十里铺地铁站" 114.312 30.5985 188
"武汉市徐州新村地铁站" 114.329 30.6373 187
"武汉市古田四路地铁站" 114.224 30.6121 186
"武汉市昙华林武胜门地铁站" 114.312 30.5596 185
"武汉市汉口火车站地铁站" 114.262 30.6216 184
"武汉市黄金口地铁站" 114.044 30.4789 183
"武汉市街道口地铁站" 114.359 30.5334 182
"武汉市取水楼地铁站" 114.278 30.598 181
"武汉市新月溪公园站地铁站" 114.513 30.4853 180
"武汉市横店地铁站" 114.306 30.8088 179
"武汉市首义路地铁站" 114.315 30.535 178
"武汉市张家湾地铁站" 114.286 30.4886 177
"武汉市古田一路地铁站" 114.189 30.6055 176
"武汉市集贤地铁站" 131.255 46.8217 175
"武汉市罗家港地铁站" 114.378 30.5992 174
"武汉市常青花园地铁站" 114.249 30.6472 173
"武汉市市民之家地铁站" 114.303 30.667 172
"武汉市建安街地铁站" 114.309 30.5113 171
"武汉市五环体育中心地铁站" 114.15 30.6464 170
"武汉市大花岭地铁站" 114.319 30.417 169
"武汉市友谊路地铁站" 114.407 30.625 168
"武汉市三角湖地铁站" 114.186 30.5222 167
"武汉市仁和路地铁站" 114.394 30.6183 166
"武汉市宏图大道地铁站" 114.286 30.6718 165
"武汉市蔡甸广场地铁站" 114.047 30.5786 164
"武汉市武钢地铁站" 114.471 30.6015 163
"武汉市四新大道地铁站" 114.208 30.534 162
"武汉市岳家嘴地铁站" 114.369 30.5834 161
"武汉市梅苑小区地铁站" 114.334 30.5379 160
"武汉市太平洋地铁站" 114.247 30.5804 159
"武汉市秀湖地铁站" 114.428 30.4534 158
"武汉市湖口地铁站" 116.258 29.7371 157
"武汉市三角路地铁站" 114.331 30.5844 156
"武汉市马湖地铁站" 114.326 30.4803 155
"武汉市汤云海路地铁站" 114.255 30.7058 154
"武汉市瑞安街地铁站" 114.325 30.5186 153
"武汉市头道街地铁站" 114.324 30.6239 152
"武汉市光谷七路地铁站" 114.54 30.4686 151
"武汉市马池地铁站" 114.242 30.6643 150
"武汉市青龙山地铁小镇地铁站" 114.341 30.3249 149
"武汉市体育中心地铁站" 114.174 30.6896 148
"武汉市兴业路地铁站" 114.154 30.3362 147
"武汉市光谷四路地铁站" 114.495 30.4598 146
"武汉市板桥地铁站" 114.32 30.48 145
"武汉市武汉站东广场地铁站" 114.433 30.6136 144
"武汉市黄浦路地铁站" 114.307 30.6331 143
"武汉市青鱼嘴地铁站" 114.356 30.5702 142
"武汉市唐家墩地铁站" 114.278 30.6236 141
"武汉市二雅路地铁站" 114.157 30.6313 140
"武汉市武汉商务区地铁站" 114.254 30.6041 139
"武汉市武昌火车站地铁站" 114.325 30.5427 138
"武汉市枫林站地铁站" 114.162 30.4005 137
"武汉市金潭路地铁站" 114.286 30.6834 136
"武汉市五里墩地铁站" 114.247 30.5602 135
"武汉市青年路地铁站" 114.273 30.5874 134
"武汉市汪家墩地铁站" 114.357 30.5915 133
"武汉市东风公司地铁站" 114.172 30.5087 132
"武汉市黄龙山路地铁站" 114.435 30.478 131
"武汉市盘龙城地铁站" 114.266 30.7064 130
"武汉市复兴路地铁站" 114.307 30.5304 129
"武汉市工人村地铁站" 114.426 30.6558 128
"武汉市双墩地铁站" 114.239 30.593 127
"武汉市小东门地铁站" 114.323 30.5496 126
"武汉市梨园地铁站" 114.376 30.581 125
"武汉市陶家岭地铁站" 114.208 30.5514 124
"武汉市车城东路地铁站" 114.184 30.4902 123
"武汉市腾龙大道地铁站" 114.283 30.727 122
"武汉市北华街地铁站" 114.337 30.3639 121
"武汉市汉西一路地铁站" 114.227 30.5922 120
"武汉市军运村地铁站" 114.299 30.4315 119
"武汉市凤凰路地铁站" 114.315 30.4935 118
"武汉市新天地铁站" 114.133 30.5584 117
"武汉市菱角湖路地铁站" 114.281 30.6134 116
"武汉市新农地铁站" 114.081 30.5661 115
"武汉市永安堂地铁站" 114.194 30.5711 114
"武汉市金融港北地铁站" 114.427 30.4676 113
"武汉市彭刘杨路地铁站" 114.31 30.5456 112
"武汉市光谷生物园地铁站" 114.548 30.4843 111
"武汉市沌阳大道地铁站" 114.193 30.4852 110
"武汉市花山河站地铁站" 114.519 30.5471 109
"武汉市大智路地铁站" 114.301 30.5949 108
"武汉市石桥地铁站" 114.329 30.3401 107
"武汉市光谷同济医院地铁站" 114.472 30.4944 106
"武汉市三店地铁站" 114.312 30.5985 105
"武汉市竹叶海地铁站" 114.17 30.6202 104
"武汉市和平公园地铁站" 114.392 30.6396 103
"武汉市临嶂大道地铁站" 114.032 30.5728 102
"武汉市沌口地铁站" 114.158 30.4957 101
"武汉市华中科技大学地铁站" 114.267 30.5887 100
"武汉市二七路地铁站" 114.319 30.6335 99
"武汉市小龟山地铁站" 114.333 30.557 98
"武汉市野芷湖地铁站" 114.348 30.4717 97
"武汉市金银湖公园地铁站" 114.176 30.6604 96
"武汉市孟家铺地铁站" 114.312 30.5985 95
"武汉市铁机路地铁站" 114.356 30.6127 94
"武汉市螃蟹岬地铁站" 114.325 30.5597 93
"武汉市七里庙地铁站" 114.238 30.5627 92
"武汉市余家头地铁站" 114.361 30.6154 91
"武汉市长港路地铁站" 114.255 30.6363 90
"武汉市园博园地铁站" 114.222 30.6322 89
"武汉市新荣地铁站" 114.339 30.6637 88
"武汉市光谷广场地铁站" 114.403 30.5113 87
"武汉市琴台地铁站" 114.273 30.5636 86
"武汉市六渡桥地铁站" 114.29 30.5821 85
"武汉市塔子湖地铁站" 114.287 30.6585 84
"武汉市烽火村地铁站" 114.305 30.5165 83
"武汉市知音地铁站" 114.323 30.5092 82
"武汉市汉阳火车站地铁站" 114.225 30.5601 81
"武汉市南太子湖地铁站" 114.226 30.483 80
"武汉市光霞地铁站" 114.3 30.4836 79
"武汉市三阳路地铁站" 114.31 30.6035 78
"武汉市武汉站西广场站地铁站" 114.428 30.6125 77
"武汉市花山新城站地铁站" 114.496 30.5656 76
"武汉市楚河汉街地铁站" 114.345 30.5658 75
"武汉市滠口新城地铁站" 114.349 30.6903 74
"武汉市玉龙路地铁站" 114.21 30.5773 73
"武汉市协子河地铁站" 114.312 30.5985 72
"武汉市建设二路地铁站" 114.383 30.63 71
"武汉市古田二路地铁站" 114.209 30.6176 70
"武汉市常码头地铁站" 114.239 30.6114 69
"武汉市堤角地铁站" 114.343 30.662 68
"武汉市宋家岗地铁站" 114.239 30.7316 67
"武汉市杨家湾地铁站" 114.389 30.5112 66
"武汉市红霞地铁站" 114.268 30.4366 65
"武汉市惠济二路地铁站" 114.296 30.6199 64
"武汉市江夏客厅地铁站" 114.328 30.395 63
"武汉市佛祖岭地铁站" 114.458 30.4639 62
"武汉市武东站地铁站" 114.437 30.4921 61
"武汉市未来一路地铁站" 114.582 30.4501 60
"武汉市滕子岗地铁站" 114.312 30.5985 59
"武汉市徐东地铁站" 114.354 30.5943 58
"武汉市青宜居地铁站" 114.427 30.6496 57
"武汉市云飞路地铁站" 114.249 30.6087 56
"武汉市洪山路地铁站" 114.351 30.5517 55
"武汉市武汉东站地铁站" 114.437 30.4921 54
"武汉市前进村地铁站" 114.263 30.5295 53
"武汉市光谷五路地铁站" 114.509 30.4595 52
"武汉市宝通寺地铁站" 114.347 30.538 51
"武汉市钟家村地铁站" 114.273 30.5553 50
"武汉市国博中心北地铁站" 114.248 30.5132 49
"武汉市洪山广场地铁站" 114.343 30.5514 48
"武汉市水果湖地铁站" 114.353 30.5585 47
"武汉市王家湾地铁站" 114.218 30.5714 46
"武汉市柏林地铁站" 113.992 30.5893 45
"武汉市周家河地铁站" 114.874 30.8425 44
"武汉市文治街地铁站" 114.341 30.508 43
"武汉市国博中心南地铁站" 114.248 30.5132 42
"武汉市司门口黄鹤楼地铁站" 114.307 30.5527 41
"武汉市白沙六路地铁站" 114.294 30.4754 40
"武汉市湖工大地铁站" 114.32 30.4896 39
"武汉市八铺街地铁站" 114.304 30.529 38
"武汉市硚口路地铁站" 114.258 30.5753 37
"武汉市汉阳客运站地铁站" 114.225 30.5601 36
"武汉市中山公园地铁站" 114.278 30.5928 35
"武汉市轻工大学地铁站" 114.241 30.6459 34
"武汉市光谷六路地铁站" 114.52 30.4891 33
"武汉市桂子湖地铁站" 114.098 30.3625 32
"武汉市后湖大道地铁站" 114.325 30.6614 31
"武汉市崇仁路地铁站" 114.268 30.5769 30
"武汉市中医药大学地铁站" 114.277 30.4538 29
"武汉市巨龙大道地铁站" 114.295 30.7085 28
"武汉市园博园北地铁站" 114.222 30.6322 27
"武汉市苗栗路地铁站" 114.298 30.6032 26
"武汉市汉口北地铁站" 114.322 30.7054 25
"武汉市汉正街地铁站" 114.275 30.5708 24
"武汉市大军山地铁站" 114.091 30.4116 23
"武汉市罗家庄地铁站" 114.31 30.6291 22
"武汉市谭鑫培公园地铁站" 114.33 30.3754 21
"武汉市积玉桥地铁站" 114.32 30.5807 20
"武汉市三层楼地铁站" 113.977 30.6043 19
"武汉市江汉路地铁站" 114.292 30.5913 18
"武汉市东亭地铁站" 114.366 30.5728 17
"武汉市小军山地铁站" 114.176 30.4241 16
"武汉市二七小路地铁站" 114.312 30.5985 15
"武汉市光谷大道地铁站" 114.349 30.5561 14
"武汉市海口三路地铁站" 114.172 30.6455 13
"武汉市额头湾地铁站" 114.162 30.6246 12
"武汉市虎泉地铁站" 114.377 30.5187 11
"武汉市武胜路地铁站" 114.275 30.5767 10
"武汉市新河街地铁站" 114.322 30.5761 9
"武汉市香港路地铁站" 114.353 30.5934 8
"武汉市天阳大道地铁站" 114.312 30.5985 7
"武汉市竹叶山地铁站" 114.292 30.6275 6
"武汉市葛店南地铁站" 114.312 30.5985 5
"武汉市建港地铁站" 114.261 30.5274 4
"武汉市江城大道地铁站" 114.23 30.5304 3
"武汉市王家墩东地铁站" 114.312 30.5985 2
"武汉市航空总部地铁站" 114.234 30.7392 1
"竹叶海 2 114.17 30.6202 1"
"云飞路 2 114.249 30.6087 3"
"三阳路 2 114.31 30.6035 7 1"
"工业四路 2 114.314 30.4845 4"
"秀湖 2 114.428 30.4534 2"
"马湖 2 114.326 30.4803 8"
"水果湖 2 114.353 30.5585 8"
"车城东路 2 114.184 30.4902 6"
"范湖 2 114.234 30.6198 2 3"
"天阳大道 2 114.312 30.5985 7"
"体育中心 2 114.174 30.6896 3"
"汤云海路 2 114.255 30.7058 7"
"竹叶山 2 114.292 30.6275 8"
"五环大道 2 114.145 30.6193 1"
"沌口 2 114.158 30.4957 16"
"青鱼嘴 2 114.356 30.5702 4"
"宗关 2 114.236 30.5851 1 3"
"宋家岗 2 114.239 30.7316 2"
"唐家墩 2 114.278 30.6236 6"
"裕福路 2 114.312 30.5985 7"
"菱角湖路 2 114.281 30.6134 3"
"鼓架山站 2 114.483 30.565 19"
"张家湾 2 114.286 30.4886 5"
"彭刘杨路 2 114.31 30.5456 5"
"二七路 2 114.319 30.6335 1"
"友谊路 2 114.407 30.625 1"
"中一路 2 114.289 30.6366 8"
"马房山 2 114.356 30.521 8"
"腾龙大道 2 114.283 30.727 7"
"中南医院 2 114.359 30.5594 8"
"首义路 2 114.315 30.535 4"
"古田三路 2 114.211 30.6015 1"
"双墩 2 114.239 30.593 3"
"新荣 2 114.339 30.6637 1"
"新路村 2 114.321 30.4508 7"
"文昌路 2 114.337 30.5035 8"
"滕子岗 2 114.312 30.5985 1"
"利济北路 2 114.276 30.5802 1"
"小龟山 2 114.333 30.557 2"
"塔子湖 2 114.287 30.6585 8"
"古田二路 2 114.209 30.6176 1"
"三角湖 2 114.186 30.5222 3"
"红钢城 2 114.419 30.6517 5"
"陶家岭 2 114.208 30.5514 3"
"马池 2 114.242 30.6643 7"
"东吴大道 2 114.162 30.629 1"
"取水楼 2 114.278 30.598 7"
"盘龙城 2 114.266 30.7064 2"
"武东站 2 114.437 30.4921 19"
"花山河站 2 114.519 30.5471 19"
"王家墩东 2 114.312 30.5985 7 2"
"佳园路 2 114.433 30.4921 2"
"小东门 2 114.323 30.5496 7"
"三店 2 114.312 30.5985 1"
"湖北大学 2 114.336 30.5837 7"
"八铺街 2 114.304 30.529 5"
"金银湖 2 114.212 30.6452 6"
"省博湖北日报 2 114.369 30.5714 8"
"豹澥 2 114.529 30.4943 11"
"街道口 2 114.359 30.5334 8 2"
"后湖大道 2 114.325 30.6614 3"
"司门口黄鹤楼 2 114.307 30.5527 5"
"大花岭 2 114.319 30.417 7"
"金银湖公园 2 114.176 30.6604 6"
"积玉桥 2 114.32 30.5807 5 2"
"烽火村 2 114.305 30.5165 5"
"纸坊大街 2 114.306 30.3465 7"
"武汉火车站 2 114.312 30.5985 4"
"市民之家 2 114.303 30.667 3"
"建安街 2 114.309 30.5113 7"
"中山公园 2 114.278 30.5928 2"
"东亭 2 114.366 30.5728 4"
"知音 2 114.323 30.5092 4"
"武汉商务区 2 114.254 30.6041 7 3"
"集贤 2 131.255 46.8217 4"
"省农科院 2 114.328 30.4903 8"
"未来三路 2 114.613 30.4766 11"
"中医药大学 2 114.277 30.4538 5"
"余家头 2 114.361 30.6154 5"
"汉阳客运站 2 114.225 30.5601 3"
"青宜居 2 114.427 30.6496 5"
"长岭山 2 114.57 30.4933 11"
"兴业路 2 114.154 30.3362 3"
"蔡甸广场 2 114.047 30.5786 4"
"惠济二路 2 114.296 30.6199 3"
"武钢 2 114.471 30.6015 5"
"王家湾 2 114.218 30.5714 4 3"
"太平洋 2 114.247 30.5804 1"
"小军山 2 114.176 30.4241 16"
"螃蟹岬 2 114.325 30.5597 7 2"
"纱帽 2 114.092 30.32 16"
"建设二路 2 114.383 30.63 5"
"循礼门 2 114.29 30.5918 1 2"
"中南路 2 114.34 30.5454 4 2"
"黄浦路 2 114.307 30.6331 8 1"
"光谷六路 2 114.52 30.4891 11"
"湾湖 2 113.839 30.3853 16"
"新庙村 2 114.019 30.5903 4"
"汉阳火车站 2 114.225 30.5601 4"
"马鹦路 2 114.263 30.5441 6"
"武汉站西广场站 2 114.428 30.6125 19"
"左岭 2 114.617 30.531 11"
"梨园 2 114.376 30.581 8"
"藏龙东街 2 114.434 30.4444 2"
"野芷湖 2 114.348 30.4717 8 7"
"光谷生物园 2 114.548 30.4843 11"
"琴台 2 114.273 30.5636 6"
"洪山广场 2 114.343 30.5514 4 2"
"复兴路 2 114.307 30.5304 4 5"
"轻工大学 2 114.241 30.6459 6"
"拦江路 2 114.276 30.5487 4"
"楚河汉街 2 114.345 30.5658 4"
"花山新城站 2 114.496 30.5656 19"
"江汉路 2 114.292 30.5913 6 2"
"汉口北 2 114.322 30.7054 1"
"四新大道 2 114.208 30.534 3"
"七里庙 2 114.238 30.5627 4"
"光霞 2 114.3 30.4836 5"
"汉口火车站 2 114.262 30.6216 2"
"赵家条 2 114.304 30.6237 8 3"
"二七小路 2 114.312 30.5985 3"
"天河机场 2 114.236 30.8077 2"
"五环体育中心 2 114.15 30.6464 6"
"额头湾 2 114.162 30.6246 1"
"巨龙大道 2 114.295 30.7085 7 2"
"葛店南 2 114.312 30.5985 11"
"工人村 2 114.426 30.6558 5"
"昙华林武胜门 2 114.312 30.5596 5"
"柏林 2 113.992 30.5893 4"
"江城大道 2 114.23 30.5304 6"
"大智路 2 114.301 30.5949 6 1"
"协子河 2 114.312 30.5985 16"
"新天 2 114.133 30.5584 4"
"罗家港 2 114.378 30.5992 4"
"杨家湾 2 114.389 30.5112 2"
"二雅路 2 114.157 30.6313 6"
"孟家铺 2 114.312 30.5985 4"
"玉龙路 2 114.21 30.5773 4"
"湖口 2 116.258 29.7371 11"
"通航机场 2 114.312 30.5985 16"
"海口三路 2 114.172 30.6455 6"
"虎泉 2 114.377 30.5187 2"
"佛祖岭 2 114.458 30.4639 2"
"三眼桥 2 114.166 30.5654 6"
"园林路 2 114.243 30.9414 4"
"武胜路 2 114.275 30.5767 6"
"武汉东站 2 114.437 30.4921 11 2"
"黄金口 2 114.044 30.4789 4"
"板桥 2 114.32 30.48 7"
"青年路 2 114.273 30.5874 2"
"仁和路 2 114.394 30.6183 4"
"新城十一路 2 114.118 30.6333 6"
"石桥 2 114.329 30.3401 6"
"徐家棚 2 114.349 30.587 8 5 7"
"龙阳村 2 114.211 30.5599 3"
"光谷五路 2 114.509 30.4595 11"
"科普公园 2 114.37 30.6233 5"
"洪山路 2 114.351 30.5517 8"
"临嶂大道 2 114.032 30.5728 4"
"古田四路 2 114.224 30.6121 1"
"长港路 2 114.255 30.6363 2"
"三层楼 2 113.977 30.6043 5"
"建港 2 114.261 30.5274 6"
"光谷五路站 2 114.509 30.4595 19"
"航空总部 2 114.234 30.7392 2"
"厂前 2 114.443 30.6206 5"
"梅苑小区 2 114.334 30.5379 4"
"硚口路 2 114.258 30.5753 1"
"红霞 2 114.268 30.4366 5"
"六渡桥 2 114.29 30.5821 6"
"瑞安街 2 114.325 30.5186 7"
"常码头 2 114.239 30.6114 7"
"杨春湖 2 114.423 30.611 4"
"谭鑫培公园 2 114.33 30.3754 7"
"老关村 2 114.225 30.5189 6 16"
"光谷同济医院 2 114.472 30.4944 11"
"国博中心北 2 114.248 30.5132 6"
"南太子湖 2 114.226 30.483 16"
"光谷七路 2 114.54 30.4686 11"
"徐州新村 2 114.329 30.6373 1"
"白沙六路 2 114.294 30.4754 5"
"罗家庄 2 114.31 30.6291 3"
"金潭路 2 114.286 30.6834 8"
"五里墩 2 114.247 30.5602 4"
"丹水池 2 114.341 30.6492 1"
"金银潭 2 114.271 30.6661 2"
"头道街 2 114.324 30.6239 1"
"大军山 2 114.091 30.4116 16"
"三角路 2 114.331 30.5844 5"
"堤角 2 114.343 30.662 1"
"新农 2 114.081 30.5661 4"
"青龙山地铁小镇 2 114.341 30.3249 7"
"黄龙山路 2 114.435 30.478 2"
"和平公园 2 114.392 30.6396 5"
"珞雄路 2 114.413 30.5081 2"
"码头潭公园 2 114.133 30.6422 6 1"
"枫林站 2 114.162 30.4005 16"
"新月溪公园站 2 114.513 30.4853 19"
"岳家嘴 2 114.369 30.5834 8 4"
"光谷广场 2 114.403 30.5113 2"
"新河街 2 114.322 30.5761 7"
"常青城 2 114.261 30.6667 2"
"小洪山 2 114.356 30.5453 8"
"滠口新城 2 114.349 30.6903 1"
"古田一路 2 114.189 30.6055 1"
"周家河 2 114.874 30.8425 16"
"武昌火车站 2 114.325 30.5427 4 7"
"黄家湖（武科大） 2 114.347 30.5218 5"
"马影河 2 114.039 30.3211 16"
"横店 2 114.306 30.8088 7"
"径河 2 114.127 30.6654 1"
"东风公司 2 114.172 30.5087 6 3"
"香港路 2 114.353 30.5934 6 7 3"
"文治街 2 114.341 30.508 8"
"国博中心南 2 114.248 30.5132 6 16"
"军运村 2 114.299 30.4315 8"
"永安堂 2 114.194 30.5711 4"
"园博园 2 114.222 30.6322 7"
"金融港北 2 114.427 30.4676 2"
"汪家墩 2 114.357 30.5915 8"
"杨汊湖 2 114.267 30.6419 6"
"湖工大 2 114.32 30.4896 7"
"宝通寺 2 114.347 30.538 2"
"苗栗路 2 114.298 30.6032 6"
"汉西一路 2 114.227 30.5922 1"
"徐东 2 114.354 30.5943 8"
"宏图大道 2 114.286 30.6718 8 2 3"
"光谷大道 2 114.349 30.5561 2"
"常青花园 2 114.249 30.6472 6 2"
"华中科技大学 2 114.267 30.5887 2"
"未来一路 2 114.582 30.4501 11"
"十里铺 2 114.312 30.5985 4"
"光谷四路 2 114.495 30.4598 11"
"铁机路 2 114.356 30.6127 4"
"广埠屯 2 114.371 30.5287 2"
"凤凰路 2 114.315 30.4935 4"
"杨园铁四院 2 114.352 30.6091 5"
"黄家湖地铁小镇 2 114.305 30.4428 8"
"桂子湖 2 114.098 30.3625 16"
"沌阳大道 2 114.193 30.4852 3"
"前进村 2 114.263 30.5295 6"
"钟家村 2 114.273 30.5553 4 6"
"汉正街 2 114.275 30.5708 6"
"江夏客厅 2 114.328 30.395 7"
"北华街 2 114.337 30.3639 7"
"武汉站东广场 2 114.433 30.6136 5"
"园博园北 2 114.222 30.6322 6 7"
"崇仁路 2 114.268 30.5769 1"
"舵落口 2 114.176 30.6155 1"
"8 : 1289 1479 2442 1047 1232 1080 6548 926 441 1387 803 1260 1660 631 767 848 1354 1405 2047 634 1707 1123 2315 5225 1362"
"11 : 194292 191365 7355 5700 1277 3453 1093 3047 3933 4948 4206 6063 30221"
"4 : 10727 17476 16701 38694 40169 2584 3469 1199 1014 1166 1605 726 1037 1027 1260 949 3593 784 4548 2104 930 8073 9409 1017 1735 11684 28872 12265 2329945 2324246 24016 23765 27277 1589 2309 2603"
"5 : 12116 10101 2895 1085 1400 3554 1391 278 1718 831 899 2475 32997 34002 1775 2467 1110 1231 1492 1349 2884 852 701 7406 3415 1222"
"6 : 1748 1644 1813 2148 1704 3764 1772 2312 816 1867 34076 31903 12477 18157 5351 970 956 1040 1897 660 1469 925 1541 1868 308 2298 0 2292 1342 6239 2371"
"7 : 23391 0 14554 2372 3816 4794 4022 0 2816 1603 5583 3220 7200 4289 4223 1319 1576 1832 1150 783 2675 1651 2623 1059 2805 3426 3760 2600 2191 1441 3564 4100"
"1 : 3075 10814 7674 412 1624 1714 1091 1160 1897 3303 1252 1093 11704 13411 888 1007 1154 1201 1128 2230 1732 1801 2338 1656 772 906 1760 1989 3144 17750 19143"
"16 : 2292 3993 6740 8153 2948 6879 5492 7268 40405 51078 111264 94703 37422"
"2 : 7619 978 5943 2788 4315 2475 922 2960 1355 1747 2678 7802 3937 781 1222 180 2936 2366 840 1155 726 1052 1222 1242 1269 1438 1365 951 16540 8594 10721 392 1579 1358 1579 1170 3169"
"3 : 1716 2183 39716 32849 3404 845 877 6186 7276 4525 1909 664 1834 941 2259 1440 999 1929 3327 5644 18645 20114 3300"
"19 : 13418 9224 1246 3028 9788 2894"
"武汉市马鹦路地铁站" 114.263 30.5441 0
```



# 生成的JSON文件内容为空

```JSON
{
    "lines": {
    },
    "stations": {
    }
}
```

- 确认参数不为空

```C++
bool JsonGenerator::generate(const HttpResponseHandler::SubwayLines &subwayLines
                             , const HttpResponseHandler::LineDistances &lineDistances
                             , const HttpResponseHandler::StationInfos &stationInfos)
{
    if (subwayLines.empty()) {
        emit errorOccured("SubwayLines is empty");
        qDebug() << "SubwayLines is empty";
        return false;
    }
    else if (lineDistances.empty()) {
        emit errorOccured("LineDistances is empty");
        qDebug() << "LineDistances is empty";
        return false;
    }
    else if (stationInfos.empty()) {
        emit errorOccured("StationInfos is empty");
        qDebug() << "StationInfos is empty";
        return false;
    }
    ...
    return true;
}
```

- 执行结果并未报错，确认是JSON序列化存在问题

- 分析后发现函数逻辑存在问题：应该是 

  ```c++
  stationArray.append(stationInfo);
  root["stations"] = stationArray;
  ```

- 而不是

  ```
  root["stations"] = stationInfo;
  ```

  



# 加入日志文件 loguru.hpp 和 log.cpp 后编译报错

- 报错信息

  ```
  D:\Qt_Projects\SubwayTransferSystem\SubwayTransferSystem\loguru.cpp:804: error: '_SH_DENYNO' was not declared in this scope
     file = _fsopen(path, mode_str, _SH_DENYNO);
                                    ^~~~~~~~~~- 
  ```

- 解决方案

  - 这个错误是因为 **Loguru** 在 Windows 平台上使用了 `_fsopen` 函数，而 `_SH_DENYNO` 是 Microsoft 特定的宏，需要包含 `<share.h>` 头文件

  ```
  #ifdef _WIN32
  #include <share.h> // 包含 _SH_DENYNO 的定义
  #endif
  ```



# JsonGenerator 生成的JSON文件在 JsonParser 中解析失败

- 问题现象

  - JsonParseError 报错为：unterminated object

- 问题分析

  - JsonParseError 中的报错 unterminated object 表示 JSON 对象没有正确闭合，可能是缺少 ']' 或者 '}'，这需要在 JSONLint 在线 JSON 校验工具上验证
  - 如果验收有效则说明生成的 JSON 文件本身并没有问题，需要考虑其他因素比如编码格式等，并且重新检查生成 JSON 文件后的流程，排查是否有可能 JsonParser 读取到的数据不完整
  - 如果验证无效则需要重新检查生成 JSON 逻辑 

- 排查过程

  - 在 JSONLint 在线校验工具中验证JSON文件的内容内容，校验结果正常，需要考虑其他因素
  - JSON 文件的编码格式确实是 UTF-8，不会将某些字符解析错误
  - 问 AI 后猜测有可能是将 JSON 文本写入文件后没有马上关闭文件，发射信号后槽函数执行，此时槽函数读取到的数据可能有一部分还没写入到硬盘

- 问题修复

  - 在 将 JSON 文本写入文件后马上关闭文件

    ```c++
    QJsonDocument jsonDocument(root);
    file.write(jsonDocument.toJson());
    file.close();
    emit generateFinished(m_fileName);
    ```

  - 结果正常

- AI 回答

  ```
  编码问题：Qt默认可能使用本地编码，而JSON需要UTF-8编码。虽然在写入文件时没有显式设置编码，但QJsonDocument的toJson()方法返回的是UTF-8编码的QByteArray，因此可能没问题。但为了确保，可以检查文件写入时是否强制使用UTF-8。
  
  文件写入未刷新或关闭：虽然QFile在析构时会自动关闭，但如果在写入后没有正确关闭文件，可能导致数据未完全写入。不过用户代码中在write之后没有立即关闭，但生成逻辑结束后应该会关闭。
  
  数据中的特殊字符：如果站点名称包含特殊字符，如未转义的双引号或反斜杠，可能导致解析错误。生成JSON时，Qt应该会自动处理这些转义，但需要确认。
  
  BOM头问题：某些情况下，文件可能包含BOM头，导致解析错误。但使用QJsonDocument生成的JSON通常不带BOM，所以可能性较低。
  
  生成过程中的逻辑错误：比如stations或lines数组的构建是否正确，是否有遗漏或格式错误。例如，检查每个line的distances数组长度是否匹配stations数量减一。
  
  用户提供的生成代码中，有一个关键检查：在写入line的distances时，检查distanceVec的大小是否等于stations数量减一。如果这里的检查不严谨，可能导致生成的JSON结构错误，例如stations数组为空时，distanceVec的大小为-1，但代码中是否有处理这种情况？
  
  另外，用户在生成JSON时，使用QFile的write方法直接写入QByteArray，可能没有错误，但最好检查是否所有数据都被正确写入，比如文件是否成功打开，是否有写入权限问题。
  
  可能的解决方案步骤：
  
  确保文件正确写入：在生成JSON后，检查文件是否确实包含完整的JSON数据，没有截断。可以在写入后读取文件内容，与预期对比。
  
  显式设置UTF-8编码：在写入文件时，使用QTextStream并设置编码为UTF-8，确保无BOM。
  
  检查数据转义：确保生成的JSON中转义字符正确，特别是站点名称中的引号或反斜杠。
  ```

  ```
  首先，用户提供的生成代码中，使用QFile的write方法写入数据后，立即发送了generateFinished信号，该信号连接的槽函数会读取并解析这个JSON文件。问题可能出现在这里，因为文件可能还没有完全写入磁盘，或者文件句柄未被释放，导致读取的内容不完整。
  
  用户提到，解决方案是在写入后立即关闭文件，再发送信号。这确实是一个关键点。Qt中，QFile在析构时会自动关闭文件，但如果在写入后没有显式关闭，可能会延迟关闭，导致读取时文件被占用或内容未更新。
  
  接着，我需要验证用户的思路是否正确。用户修改后的代码在写入后调用了file.close()，确保数据写入磁盘并释放文件句柄，这样后续读取时就能得到最新的内容。这应该能解决解析错误的问题。
  
  另外，用户可能在生成JSON文件时使用了QTextStream并设置编码为UTF-8，避免编码问题。同时，写入后验证JSON的步骤也能帮助捕捉潜在的错误，比如数据不匹配或格式问题。
  
  总结用户的解决过程：生成JSON后立即关闭文件，确保数据完全写入，再发送信号触发解析，这样就能避免读取到不完整的数据，从而解决“unterminated object”错误。
  
  问题原因
  在生成 JSON 文件后未及时关闭文件句柄，导致后续解析时读取到未完全写入的数据。具体流程如下：
  
  通过 file.write(jsonDocument.toJson()) 写入数据，但未调用 file.close()。
  立即通过信号 generateFinished(m_fileName) 触发解析操作。
  解析时文件可能仍被占用或数据未完全写入磁盘，导致读取到不完整的 JSON 内容。
  ```

  ```
  关键原理
  文件写入缓冲：
  Qt 的 write() 方法会将数据写入内存缓冲区，而非立即同步到磁盘。调用 close() 或 flush() 会强制将缓冲区数据写入磁盘。
  
  文件句柄占用：
  在文件未关闭时，其他进程（或同一进程的后续操作）可能无法完整读取文件内容，尤其是大文件。
  
  信号触发时机：
  必须在文件完全写入并释放后，再通知外部程序进行解析，避免读写冲突。
  ```




# BFS搜索的路径不对

## 问题表现

- BFS搜索的路径不是最少边路径

- Graph

- ```
  "F" QSharedPointer(0x3430d00)
  "B" QSharedPointer(0x3431b40)
  "C" QSharedPointer(0x3431080)
  "E" QSharedPointer(0x3431a80)
  "A" QSharedPointer(0x3431440)
  "D" QSharedPointer(0x3431000)
  "1 A B E"
  "2 C B D"
  "3 E D F"
  "F -> (D, 4)"
  "B -> (A, 3) -> (E, 4) -> (C, 2) -> (D, 3)"
  "C -> (B, 2)"
  "E -> (B, 4) -> (D, 4)"
  "A -> (B, 3)"
  "D -> (B, 3) -> (E, 4) -> (F, 4)"
  "F" QSharedPointer(0x3430d00)
  "B" QSharedPointer(0x3431b40)
  "C" QSharedPointer(0x3431080)
  "E" QSharedPointer(0x3431a80)
  "A" QSharedPointer(0x3431440)
  "D" QSharedPointer(0x3431000)
  "1 A B E"
  "2 C B D"
  "3 E D F"
  "F -> (D, 4)"
  "B -> (A, 3) -> (E, 4) -> (C, 2) -> (D, 3)"
  "C -> (B, 2)"
  "E -> (B, 4) -> (D, 4)"
  "A -> (B, 3)"
  "D -> (B, 3) -> (E, 4) -> (F, 4)"
  "D"  ->  "F"
  "A"  ->  "B"
  "B"  ->  "C"
  "B"  ->  "E"
  "E"  ->  "D"
  15
  "A B E D F"
  ```

- BFS算法

- ```C++
  SubwayGraph::PathInfo SubwayGraph::bfs(const QString& start, const QString& end) const
  {
      // validate start and end are in graph
      // to do
      if (start == end) {
          PathInfo::Path path;
          path.push_back(start);
          return PathInfo(0, path);
      }
  
      // init
      QHash<QString, bool> visited;
      // QPair<QString, QString>::second -> QPair<QString, QString>::first
      QHash<QString, QString> parent;
      // {parent::item} -> distance
      QHash<QPair<QString, QString>, int> distances;
      QQueue<QString> nodeQueue;
      for (auto station = m_stations.begin(); station != m_stations.end(); ++station) {
          visited[station.key()] = false;
      }
      nodeQueue.push_back(start);
  
      while (nodeQueue.isEmpty() != true) {
          QString node = nodeQueue.front();
          nodeQueue.pop_front();
          if (node == end) {
              const PathInfo& pathInfo = generatePathInfo(start, end, parent, distances);
              return pathInfo;
          }
          if (visited[node])
              continue;
  
          visited[node] = true;
  
          for (auto edge = m_graph[node].begin(); edge != m_graph[node].end(); ++edge) {
              if (visited[edge->toNode->param.name]) {
                  continue;
              }
              nodeQueue.push_back(edge->toNode->param.name);
              parent[edge->toNode->param.name] = node;
              auto nodePair = qMakePair<QString, QString>(node, edge->toNode->param.name);
              QHash<QPair<QString, QString>, int>::iterator distanceIterator = distances.find(nodePair);
              if (distanceIterator != distances.end()) {
                  distanceIterator.value() = edge->distance;
              }
              else {
                  distances[nodePair] = edge->distance;
              }
          }
      }
  
      return PathInfo(-1, PathInfo::Path());
  }
  ```


## 问题表现

- 1

- ```
  "A" QSharedPointer(0x3561900)
  "F" QSharedPointer(0x3560d40)
  "B" QSharedPointer(0x3561080)
  "E" QSharedPointer(0x3561440)
  "C" QSharedPointer(0x3561940)
  "D" QSharedPointer(0x3561600)
  "1 A B E"
  "2 C B D"
  "3 E D F"
  "A -> (B, 3)"
  "F -> (D, 4)"
  "B -> (A, 3) -> (E, 4) -> (C, 2) -> (D, 3)"
  "E -> (B, 4) -> (D, 4)"
  "C -> (B, 2)"
  "D -> (B, 3) -> (E, 4) -> (F, 4)"
  "A" QSharedPointer(0x3561900)
  "F" QSharedPointer(0x3560d40)
  "B" QSharedPointer(0x3561080)
  "E" QSharedPointer(0x3561440)
  "C" QSharedPointer(0x3561940)
  "D" QSharedPointer(0x3561600)
  "1 A B E"
  "2 C B D"
  "3 E D F"
  "A -> (B, 3)"
  "F -> (D, 4)"
  "B -> (A, 3) -> (E, 4) -> (C, 2) -> (D, 3)"
  "E -> (B, 4) -> (D, 4)"
  "C -> (B, 2)"
  "D -> (B, 3) -> (E, 4) -> (F, 4)"
  "D"  ->  "F"
  "A"  ->  "B"
  "B"  ->  "E"
  "B"  ->  "C"
  "B"  ->  "D"
  10
  "A B D F"
  ```

- ```
  SubwayGraph::PathInfo SubwayGraph::bfs(const QString& start, const QString& end) const
  {
      // validate start and end are in graph
      // to do
      if (start == end) {
          PathInfo::Path path;
          path.push_back(start);
          return PathInfo(0, path);
      }
  
      // init
      QHash<QString, bool> visited;
      // QPair<QString, QString>::second -> QPair<QString, QString>::first
      QHash<QString, QString> parent;
      // {parent::item} -> distance
      QHash<QPair<QString, QString>, int> distances;
      QQueue<QString> nodeQueue;
      for (auto station = m_stations.begin(); station != m_stations.end(); ++station) {
          visited[station.key()] = false;
      }
      nodeQueue.push_back(start);
  
      while (nodeQueue.isEmpty() != true) {
          QString node = nodeQueue.front();
          nodeQueue.pop_front();
          if (node == end) {
              const PathInfo& pathInfo = generatePathInfo(start, end, parent, distances);
              return pathInfo;
          }
          // neccesary
          if (visited[node])
              continue;
  
          visited[node] = true;
  
          for (auto edge = m_graph[node].begin(); edge != m_graph[node].end(); ++edge) {
              if (visited[edge->toNode->param.name]) {
                  continue;
              }
              if (parent.find(edge->toNode->param.name) == parent.end()) {
                  parent[edge->toNode->param.name] = node;
              }
              nodeQueue.push_back(edge->toNode->param.name);          
              auto nodePair = qMakePair<QString, QString>(node, edge->toNode->param.name);
              QHash<QPair<QString, QString>, int>::iterator distanceIterator = distances.find(nodePair);
              if (distanceIterator != distances.end()) {
                  distanceIterator.value() = edge->distance;
              }
              else {
                  distances[nodePair] = edge->distance;
              }
          }
      }
  
      return PathInfo();
  }
  ```

  
