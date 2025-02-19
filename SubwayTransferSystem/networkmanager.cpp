#include "networkmanager.h"
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

HttpResponseHandler::HttpResponseHandler(QObject *parent)
    : QObject(parent)
{

}

void HttpResponseHandler::onRequestFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QString context(responseData);
        qDebug() << context;
    }
    else {
        emit errorOccured(reply->errorString());
    }
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    (void)connect(&m_manager, &HttpRequestManager::requestFinished, &m_handler, &HttpResponseHandler::onRequestFinished);

}

void NetworkManager::request(const QUrl &url/*, NetworkManager::RequestType type, const QByteArray &data*/)
{
    /*
    switch (type) {
    case NetworkManager::RequestType::Get:
        m_manager.sendGetRequest(url);
        break;
    case NetworkManager::RequestType::Post:
        m_manager.sendPostRequest(url, data);
        break;
    default:
        break;
    }
    */
    m_manager.sendGetRequest(url);
}
