#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QString>

class SubwayGraph;

class JsonParser
{
public:
    static JsonParser& getJsonParserInstance();

    QString getFileName() const { return m_fileName; }
    void setFileName(const QString& fileName) { m_fileName = fileName; }

    bool parse(QString& err_msg);

private:
    JsonParser();

    SubwayGraph* m_subwayGraph;
    QString m_fileName;
};

#endif // JSONPARSER_H
