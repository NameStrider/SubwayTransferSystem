#include "jsonparser.h"
#include "loguru.hpp"
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

JsonParser::JsonParser()
    : m_fileName(":/resource/data/wuhanmetro.json")
{}

JsonParser &JsonParser::getJsonParserInstance()
{
    static JsonParser parser;
    return parser;
}

bool JsonParser::parse(QString fileName)
{
    m_fileName = fileName;

    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) != true) {
        QString err_msg = QString("parse failed as JsonParser can not open file %s").arg(m_fileName);
        emit errorOccured(err_msg);
        LOG_F(ERROR, err_msg.toStdString().c_str());
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close(); // 提前关闭文件

    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(fileData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QString err_msg = parseError.errorString();
        emit errorOccured(err_msg);
        LOG_F(ERROR, err_msg.toStdString().c_str());
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

    LOG_SCOPE_F(INFO, __FUNCTION__);
    emit parseFinished(m_lineNames, m_lineDistances, m_stationNodeParams);
    return true;
}

void JsonParser::clear()
{
    m_lineNames.clear();
    m_lineDistances.clear();
    m_stationNodeParams.clear();
}
