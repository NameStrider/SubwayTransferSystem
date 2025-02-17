#include "jsonparser.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

JsonParser::JsonParser()
    : m_fileName(":/resource/data/wuhanmetro.json")
    , m_subwayGraph(nullptr)
{}

JsonParser &JsonParser::getJsonParserInstance()
{
    static JsonParser parser;
    return parser;
}

bool JsonParser::parse(QString &err_msg)
{
    if (m_subwayGraph == nullptr) {
        err_msg = QString("parse failed as SubwayGraph object can not be null");
        return false;
    }

    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) != true) {
        err_msg = QString("parse failed as JsonParser can not open file %s").arg(m_fileName);
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close(); // 提前关闭文件

    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(fileData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        err_msg = parseError.errorString();
        return false;
    }

    QJsonObject root = jsonDocument.object();
    QJsonArray stations = root["stations"].toArray();
    for (const QJsonValue& value : stations) {
        QJsonObject obj = value.toObject();
        QString name = obj["name"].toString();
        double latitude = obj["latitude"].toDouble();
        double longitude = obj["longitude"].toDouble();
        int stayTime = obj["stayTime"].toInt();
        QSet<int> belongingLines;
        QJsonArray belongingLinesArray = obj["belongingLines"].toArray();
        for (const QJsonValue& belongingLine : belongingLinesArray) {
            belongingLines.insert(belongingLine.toInt());
        }
        StationNodeParam param(name,  longitude, latitude, stayTime, belongingLines);
        m_stationNodeParams.push_back(param);
    }

    QJsonArray lines = root["lines"].toArray();
    for (const QJsonValue& value : lines) {
        QJsonObject obj = value.toObject();
        int id = obj["id"].toInt();

        QJsonArray namesArray = obj["stations"].toArray();
        QVector<QString> names;
        for (const QJsonValue& name : namesArray) {
            names.push_back(name.toString());
        }
        m_lineNames[id] = names;

        QJsonArray distancesArray = obj["distances"].toArray();
        QVector<int> distances;
        for (const QJsonValue& distance : distancesArray) {
            distances.push_back(distance.toInt());
        }
        m_lineDistances[id] = distances;
    }

    emit parseFinished(m_lineNames, m_lineDistances, m_stationNodeParams);
    return true;
}

void JsonParser::clear()
{
    m_lineNames.clear();
    m_lineDistances.clear();
    m_stationNodeParams.clear();
}
