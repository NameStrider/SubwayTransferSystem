#ifndef JSONGENERATOR_H
#define JSONGENERATOR_H

#include "networkmanager.h"

class JsonGenerator : public QObject
{
    Q_OBJECT

public:
    static JsonGenerator &getJsonGenerator();

    QString getFileName() const { return m_fileName; }
    void setFileName(const QString& fileName) { m_fileName = fileName; }

public slots:
    bool generate(const HttpResponseHandler::SubwayLines& subwayLines, const HttpResponseHandler::StationInfos& stationInfos);

private:
    JsonGenerator();

    QString m_fileName;
};

#endif // JSONGENERATOR_H
