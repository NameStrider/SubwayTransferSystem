#ifndef LINELIST_H
#define LINELIST_H

#include "stationnode.h"
#include <QList>
#include <QTime>
#include <QSharedPointer>

class LineList
{
public:
    LineList();
    LineList(int lineId
             , const QTime& firstDepartingTime = QTime(0, 0, 0)
             , const QTime& lastDepartingTime = QTime(0, 0, 0));

    bool isValid() const;

    QSharedPointer<StationNode> findNode(const QString& name) const;

    QSharedPointer<StationNode> findPreNode(const QString& name) const;
    QSharedPointer<StationNode> findPreNode(const QSharedPointer<StationNode>& node) const;
    QSharedPointer<StationNode> findNextNode(const QString& name) const;
    QSharedPointer<StationNode> findNextNode(const QSharedPointer<StationNode>& node) const;

    int size() const { return m_line.size(); }
    int lineId() const { return m_lineId; }
    QTime firtDepartingTime() const { return m_firstDepartingTime; }
    QTime lastDepartingTime() const { return m_lastDepartingTime; }
    QSharedPointer<StationNode> start() const { return m_line.front(); }
    QSharedPointer<StationNode> end() const { return m_line.back(); }
    const QList<QSharedPointer<StationNode>>& getLineList() const { return m_line; }

    void addNode(const QSharedPointer<StationNode>& node, const QString& preName = "");
    void removeNode(const QString& name);

    void setLineId(int lineId) { m_lineId = lineId; }
    void setFirstDepartingTime(const QTime& time) { m_firstDepartingTime = time; }
    void setLastDepartingTime(const QTime& time) { m_lastDepartingTime = time; }

private:
    int m_lineId;
    QTime m_firstDepartingTime;
    QTime m_lastDepartingTime;
    QList<QSharedPointer<StationNode>> m_line;
};

#endif // LINELIST_H
