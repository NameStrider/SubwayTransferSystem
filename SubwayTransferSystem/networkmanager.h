#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QNetworkReply>
#include <QNetworkAccessManager>

class HttpRequestManager : public QObject
{
    Q_OBJECT

public:
    explicit HttpRequestManager(QObject* parent = nullptr);

public slots:
    void sendGetRequest(const QUrl& url);
    void sendPostRequest(const QUrl& url, const QByteArray& data);

signals:
    void requestFinished(QNetworkReply* reply);
    void errorOccured(QString error);

private:
    QNetworkAccessManager m_manager;
};

class HttpResponseHandler : public QObject
{
    Q_OBJECT

public:
    explicit HttpResponseHandler(QObject* parent = nullptr);

public slots:
    void onRequestFinished(QNetworkReply* reply);
};

class NetworkManager : QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject* parent = nullptr);

    void request();


private:
    HttpRequestManager m_manager;
    HttpResponseHandler m_handler;
};

#endif // HTTPREQUESTMANAGER_H
