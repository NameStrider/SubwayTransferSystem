#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "httpresponseparser.h"

#include <QNetworkReply>
#include <QNetworkAccessManager>

// 网络请求和回复都是异步，因此使用信号和槽机制
// HttpResponseHandler 必须在前面以便于 HttpRequestManager 能识别 SubwayLines
class HttpResponseHandler : public QObject
{
    Q_OBJECT

public:
    using SubwayLines = QHash<int, QVector<QString>>;
    // using StationParams = QHash<QString, >

    explicit HttpResponseHandler(QObject* parent = nullptr);

    const SubwayLines& subwayLines() const { return m_subwayLines; }

    SubwayLines& subwayLines() { return const_cast<SubwayLines&>(m_subwayLines); }

    void printSubwayLines() const;

public slots:
    void onRequestFinished(QNetworkReply* reply);

signals:
    void subwayLineParsed(const SubwayLines& subwayLines);
    void errorOccured(QString error);

private:
    QByteArray m_buffer;
    QString m_context;
    SubwayLines m_subwayLines;

    HttpResponseParserFactory m_parserFactory;
};

class HttpRequestManager : public QObject
{
    Q_OBJECT

public:
    explicit HttpRequestManager(QObject* parent = nullptr);

public slots:
    void sendGetRequest(const QUrl& url);
    void sendPostRequest(const QUrl& url, const QByteArray& data);

public slots:
    void requestStation(const HttpResponseHandler::SubwayLines& subwayLines);

signals:
    void requestFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager m_manager;
};

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject* parent = nullptr);

    const HttpRequestManager& manager() const { return m_manager; }
    const HttpResponseHandler& handler() const { return m_handler; }

public slots:
    void request(const QUrl& url);

signals:
    void errorOccured(QString error);

private:
    HttpRequestManager m_manager;
    HttpResponseHandler m_handler;
};

#endif // NETWORKMANAGER_H
