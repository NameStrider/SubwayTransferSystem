#include "jsongenerator.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

JsonGenerator &JsonGenerator::getJsonGenerator()
{
    static JsonGenerator generator;
    return generator;
}

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

    // Qfile object automatically close file when destroyed
    QFile file(m_fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) != true) {
        QString err_msg = QString("can not open file %1").arg(m_fileName);
        emit errorOccured(err_msg);
        qDebug() << err_msg;
        return false;
    }

    QString err_msg;
    // json file frame
    QJsonObject root;
    QJsonArray stationArray;
    QJsonArray lineArray;

    // write stations
    for (auto it = stationInfos.begin(); it != stationInfos.end(); ++it) {
        if (it.key() != it.value().basicInfo.name) {
            err_msg = QString("generate json file failed as param StationInfo %1 invalid").arg(it.key());
            emit errorOccured(err_msg);
            qDebug() << err_msg;
            return false;
        }
        else {
            QJsonObject stationInfo;
            stationInfo["name"] = it.value().basicInfo.name;
            stationInfo["longitude"] = it.value().basicInfo.longitude;
            stationInfo["latitude"] = it.value().basicInfo.latitude;
            stationInfo["stayTime"] = it.value().basicInfo.stayTime;
            QJsonArray belongingLines;
            for (int lineId : it.value().basicInfo.belongingLines) {
                belongingLines.append(lineId);
            }
            stationInfo["belongingLines"] = belongingLines;
            stationArray.append(stationInfo);
        }
    }
    root["stations"] = stationArray;

    // write lines
    for (auto it = subwayLines.begin(); it != subwayLines.end(); ++it) {
        int lineId = it.key();
        QJsonObject lineInfo;
        lineInfo["id"] = lineId;
        QJsonArray stations;
        for (const QString& station : it.value()) {
            stations.append(station);
        }
        lineInfo["stations"] = stations;

        QJsonArray distances;
        if (lineDistances.find(lineId) == lineDistances.end()) {
            err_msg = QString("line %1 can not found in LineDistances").arg(lineId);
            qDebug() << err_msg;
            return false;
        }
        const QVector<int>& distanceVec = lineDistances[lineId];
        if (it.value().size() != distanceVec.size() + 1) {
            err_msg = QString("count is mismatched between SubwayLines and LineDistances while lineId is %1").arg(lineId);
            qDebug() << err_msg;
            return false;
        }
        for (int distance : distanceVec) {
            distances.append(distance);
        }
        lineInfo["distances"] = distances;

        lineArray.append(lineInfo);
    }
    root["lines"] = lineArray;

    QJsonDocument jsonDocument(root);
    file.write(jsonDocument.toJson());
    file.close();

    emit generateFinished(m_fileName);
    return true;
}

JsonGenerator::JsonGenerator()
    : m_fileName("D:\\Qt_Projects\\wuhan_metro.json")
{}
