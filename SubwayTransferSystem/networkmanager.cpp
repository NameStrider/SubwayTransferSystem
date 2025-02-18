#include "networkmanager.h"

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

HttpResponseHandler::HttpResponseHandler(QObject *parent)
    : QObject(parent)
{

}

void HttpResponseHandler::onRequestFinished(QNetworkReply *reply)
{

}
