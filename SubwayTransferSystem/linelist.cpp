#include "linelist.h"

LineList::LineList()
    : m_lineId(0)
    , m_firstDepartingTime(QTime(0, 0, 0))
    , m_lastDepartingTime(QTime(0, 0, 0))
{}

LineList::LineList(int lineId, const QTime& firstDepartingTime, const QTime& lastDepartingTime)
    : m_lineId(lineId)
    , m_firstDepartingTime(firstDepartingTime)
    , m_lastDepartingTime(lastDepartingTime)
{}

QSharedPointer<StationNode> LineList::findNode(const QString &name) const
{
    for (auto node = m_line.begin(); node != m_line.end(); ++node) {
        if ((*node)->param.name == name)
            return *node;
    }
    return QSharedPointer<StationNode>();
}

bool LineList::isValid() const
{
    // 一条线路至少有两个站点
    // 返回 false 一般说明该线路保留未规划
    return m_line.size() > 1 && m_lineId >= MIN_LINE_ID && m_lineId <= MAX_LINE_ID;
}

void LineList::addNode(const QSharedPointer<StationNode>& node, const QString& preName)
{
    // 插入链表的三种情况
    if (preName.isEmpty()) {
        m_line.push_front(node);
        return;
    }
    else if (preName == end()->param.name) {
        m_line.push_back(node);
        return;
    }

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

void LineList::removeNode(const QString &name)
{
    for (auto node = m_line.begin(); node != m_line.end(); ++node) {
        if ((*node)->param.name == name) {
            m_line.erase(node);
            break;
        }
    }
}
