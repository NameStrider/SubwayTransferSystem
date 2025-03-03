#include "networkmanager.h"
#include "const.h"
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

void HttpRequestManager::sendPostRequest(const QUrl &url, const QByteArray& data)
{
    QNetworkRequest request(url);
    m_manager.post(request, data);
}

void HttpRequestManager::requestStation(const HttpResponseHandler::SubwayLines& subwayLines)
{
    QUrl url("http://restapi.amap.com/v3/place/text?key=c079b555e5bce39d8b5192fdc3ec5ec3&keywords=循礼门&types=&city=武汉&children=1&offset=1&page=1&extensions=all");
}

HttpResponseHandler::HttpResponseHandler(QObject *parent)
    : QObject(parent)
    , m_parserFactory(parent, *this)
{

}

void HttpResponseHandler::printSubwayLines() const
{
    for (auto line = m_subwayLines.begin(); line != m_subwayLines.end(); ++line) {
        QStringList context;
        context << QString::number(line.key()) << ":";
        for (const auto& name : line.value()) {
            context << name;
        }
        qDebug() << context.join(" ");
    }
}

void HttpResponseHandler::onRequestFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // to be improved
        m_buffer.clear();
        m_context.clear();
        m_buffer = reply->readAll();
        // can not find correct encoding format yet
        QString html(m_buffer);
        m_context = std::move(html);
        qDebug() << m_context;

        if (reply->request().url().toString() == DEFAULT_WUHAN_METRO_REQUEST_URL) {
            QString err_msg;
            HttpResponseParserBase& parser = m_parserFactory.getHttpResponseParser(HttpResponseParserType::WhApp);
            parser.parse(m_context, err_msg);
            // printSubwayLines();
            // emit subwayLineParsed(m_subwayLines);
        }
        else {

        }
    }
    else {
        QString err_msg = QString("net error: %1 (%2)").arg(reply->errorString()).arg(reply->error());
        emit errorOccured(err_msg);
    }
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    (void)connect(&m_manager, &HttpRequestManager::requestFinished, &m_handler, &HttpResponseHandler::onRequestFinished);
    (void)connect(&m_handler, &HttpResponseHandler::errorOccured, this, &NetworkManager::errorOccured);
    (void)connect(&m_handler, &HttpResponseHandler::subwayLineParsed, &m_manager, &HttpRequestManager::requestStation);
}

void NetworkManager::request(const QUrl &url)
{
    m_manager.sendGetRequest(url);
}
