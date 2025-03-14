#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "const.h"
#include "httpresponseparser.h"
#include <QNetworkReply>
#include <QNetworkAccessManager>

struct StationInfo
{
    struct BasicInfo
    {
        QString name;
        int stayTime;
        double longitude;
        double latitude;
        QSet<int> belongingLines;
    };

    struct ExtendedInfo
    {
        QSet<QChar> exits;

    };

    BasicInfo basicInfo;
    ExtendedInfo extendedInfo;
};

// HTTP request and response are both asynchronous, so use signal-slot mechanism
// HttpResponseHandler must be front of HttpRequestManager to enable complier correctly parse SubwayLines
// parse and send requests simultaneously or wait for parsing finished then send requests?
// adopt plan 2 as parse and send requests simultaneously by regular expression is hard

class HttpResponseHandler : public QObject
{
    Q_OBJECT

public:
    using SubwayLines = QHash<int, QVector<QString>>;  
    using LineDistances = QHash<int, QVector<int>>; // unit: meter
    using StationInfos = QHash<QString, StationInfo>;

    explicit HttpResponseHandler(QObject* parent = nullptr);

    const SubwayLines& subwayLines() const { return m_subwayLines; }

    SubwayLines& subwayLines() { return const_cast<SubwayLines&>(m_subwayLines); }

    const StationInfos& stationInfos() const { return m_stationInfos; }

    StationInfos& stationInfos() { return const_cast<StationInfos&>(m_stationInfos); }

    const LineDistances& lineDistances() const { return m_lineDistances; }

    HttpResponseParserFactory& getHttpResponseParserFactory() const {
        return const_cast<HttpResponseParserFactory&>(m_parserFactory);
    }

    // this function can not be included in HttpResponseParser
    // emit signal httpRequestPending at interval
    void schedulePendingHttpRequest(const QSet<QUrl>& pendingUrls, int interval);

    void clear();

public slots:
    void onRequestFinished(QNetworkReply* reply);
    void onParseFinished();

signals:
    void errorOccured(QString error);   
    void httpRequestPending(const QUrl& url);
    void httpRequestPending(QSet<QUrl> urls);
    void handleFinished(const SubwayLines& subwayLines, const LineDistances& lineDistances, const StationInfos& staionInfos);

private:  
    // do not save template data
    SubwayLines m_subwayLines;
    LineDistances m_lineDistances;
    StationInfos m_stationInfos;
    QSet<QUrl>::const_iterator m_urlIndex;
    QSet<QUrl> m_pendingUrls;

    HttpResponseParserFactory m_parserFactory;
};

class HttpRequestManager : public QObject
{
    Q_OBJECT

public:
    explicit HttpRequestManager(QObject* parent = nullptr);

public slots:
    void sendGetRequest(const QUrl& url);
    void sendGetRequest(QSet<QUrl> urls);
    void sendPostRequest(const QUrl& url, const QByteArray& data);

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
    void requestFinished(const HttpResponseHandler::SubwayLines& subwayLines
                         , const HttpResponseHandler::LineDistances& lineDistances
                         , const HttpResponseHandler::StationInfos& stationInfos);

private:
    HttpRequestManager m_manager;
    HttpResponseHandler m_handler;
};

void printSubwayLines(const HttpResponseHandler::SubwayLines& subwayLines);
void printStationInfos(const HttpResponseHandler::StationInfos& stationInfos);
void printLineDistances(const HttpResponseHandler::LineDistances& lineDistances);

#endif // NETWORKMANAGER_H
