#include "networkmanager.h"
#include <QTimer>
#include <QUrlQuery>
#include <QGeoCoordinate>
#include <QDebug>

HttpRequestManager::HttpRequestManager(QObject* parent)
    : QObject(parent)
{
    (void)connect(&m_manager, &QNetworkAccessManager::finished, this, &HttpRequestManager::requestFinished);
}

void HttpRequestManager::sendGetRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    m_manager.get(request);
}

void HttpRequestManager::sendGetRequest(QSet<QUrl> urls)
{
    for (const QUrl& url : urls) {
        sendGetRequest(url);
    }
}

void HttpRequestManager::sendPostRequest(const QUrl &url, const QByteArray& data)
{
    QNetworkRequest request(url);
    m_manager.post(request, data);
}

HttpResponseHandler::HttpResponseHandler(QObject *parent)
    : QObject(parent)
    , m_parserFactory(parent, *this)
{
    (void)connect(&m_parserFactory, &HttpResponseParserFactory::parseFinished, this, &HttpResponseHandler::onParseFinished);
}

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

void HttpResponseHandler::clear()
{
    m_subwayLines.clear();
    m_stationInfos.clear();
}

void HttpResponseHandler::onRequestFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // to be improved
        QByteArray buffer = reply->readAll();
        QString context(buffer);
        // qDebug() << context;

        QString err_msg;
        if (reply->request().url().toString() == DEFAULT_WUHAN_METRO_REQUEST_URL) {         
            HttpResponseParserBase& parser = m_parserFactory.getHttpResponseParser(HttpResponseParserType::WhApp);
            parser.parse(context, err_msg);
            printSubwayLines(m_subwayLines);
        }
        else if (reply->request().url().toString().contains(BDMAP_DOMAIN_NAME)) {
            // need specially process as response content of BaiDu map does not include address
            QUrlQuery query(reply->request().url());
            QString address = query.queryItemValue("address");
            if (address.isEmpty()) {
                return;
            }
            HttpResponseParserBase::appendTagToString(address, context);
            HttpResponseParserBase& parser = m_parserFactory.getHttpResponseParser(HttpResponseParserType::BDMap);
            parser.parse(context, err_msg);
        }
        else {
            HttpResponseParserBase& parser = m_parserFactory.getHttpResponseParser(HttpResponseParserType::WhAPPDetail);
            parser.parse(context, err_msg);
        }
        if (!err_msg.isEmpty()) {
            qDebug() << err_msg;
        }
    }
    else {
        QString err_msg = QString("net error: %1 (%2)").arg(reply->errorString()).arg(reply->error());
        emit errorOccured(err_msg);
    }

    reply->deleteLater();
}

void HttpResponseHandler::onParseFinished()
{
    // calculate distances between stations of one line
    for (auto line = m_subwayLines.begin(); line != m_subwayLines.end(); ++line) {
        int lineId = line.key();
        if (m_lineDistances.find(lineId) == m_lineDistances.end()) {
            m_lineDistances.insert(lineId, QVector<int>());
        }
        const QVector<QString>& stations = line.value();
        for (int i = 0; i < stations.size() - 1; ++i) {
            const QString& fromStation = stations[i];
            const QString& toStation = stations[i + 1];
            if (m_stationInfos.find(fromStation) == m_stationInfos.end()
                || m_stationInfos.find(toStation) == m_stationInfos.end()) {
                // log
                qDebug() << __FUNCTION__ << __LINE__ << "error";
            }
            const StationInfo::BasicInfo& fromStationBasicInfo = m_stationInfos[fromStation].basicInfo;
            const StationInfo::BasicInfo& toStationBasicInfo = m_stationInfos[toStation].basicInfo;
            QGeoCoordinate from(fromStationBasicInfo.latitude, fromStationBasicInfo.longitude);
            QGeoCoordinate to(toStationBasicInfo.latitude, toStationBasicInfo.longitude);
            int distance = from.distanceTo(to);
            m_lineDistances[lineId].push_back(distance);
        }
    }

    printStationInfos(m_stationInfos);
    printLineDistances(m_lineDistances);

    emit handleFinished(m_subwayLines, m_lineDistances, m_stationInfos);
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    (void)connect(&m_manager, &HttpRequestManager::requestFinished, &m_handler, &HttpResponseHandler::onRequestFinished);
    (void)connect(&m_handler, &HttpResponseHandler::errorOccured, this, &NetworkManager::errorOccured);
    (void)connect(&m_handler, &HttpResponseHandler::handleFinished, this, &NetworkManager::requestFinished);
    (void)connect(&m_handler, QOverload<const QUrl&>::of(&HttpResponseHandler::httpRequestPending)
                  , &m_manager, QOverload<const QUrl&>::of(&HttpRequestManager::sendGetRequest));
}

void NetworkManager::request(const QUrl &url)
{
    m_manager.sendGetRequest(url);
}

void printSubwayLines(const HttpResponseHandler::SubwayLines& subwayLines)
{
    for (auto line = subwayLines.begin(); line != subwayLines.end(); ++line) {
        QStringList context;
        context << QString::number(line.key()) << ":";
        for (const auto& name : line.value()) {
            context << name;
        }
        qDebug() << context.join(" ");
    }
}

void printStationInfos(const HttpResponseHandler::StationInfos& stationInfos)
{
    for (auto stationInfo = stationInfos.begin(); stationInfo != stationInfos.end(); ++stationInfo) {
        QStringList context;
        context << stationInfo.value().basicInfo.name
                << QString::number(stationInfo.value().basicInfo.stayTime)
                << QString::number(stationInfo.value().basicInfo.longitude)
                << QString::number(stationInfo.value().basicInfo.latitude);
        for (auto belongingLine : stationInfo.value().basicInfo.belongingLines) {
            context << QString::number(belongingLine);
        }

        qDebug() << context.join(" ");
    }
}

void printLineDistances(const HttpResponseHandler::LineDistances& lineDistances)
{
    for (auto line = lineDistances.begin(); line != lineDistances.end(); ++line) {
        QStringList context;
        context << QString::number(line.key()) << ":";
        for (int distance : line.value()) {
            context << QString::number(distance);
        }

        qDebug() << context.join(" ");
    }
}
