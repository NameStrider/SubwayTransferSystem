#ifndef JSONPARSER_H
#define JSONPARSER_H

#include "subwaygraph.h"

class JsonParser : QObject
{
    Q_OBJECT

public:
    static JsonParser& getJsonParserInstance();

    QString getFileName() const { return m_fileName; }
    void setFileName(const QString& fileName) { m_fileName = fileName; }

public slots:
    bool parse(QString& err_msg);
    void clear();

signals:
    void errorOccured(QString err_msg);
    void parseFinished(const SubwayGraph::LineNames& lineNames
                       , const SubwayGraph::LineDistances& lineDistances
                       , const SubwayGraph::StationNodeParams& nodeParams);   

public:
    JsonParser();

    QString m_fileName;
    SubwayGraph::LineNames m_lineNames;
    SubwayGraph::LineDistances m_lineDistances;
    SubwayGraph::StationNodeParams m_stationNodeParams;
};

#endif // JSONPARSER_H
