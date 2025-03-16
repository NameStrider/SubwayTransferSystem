#ifndef HTTPRESPONSEPARSER_H
#define HTTPRESPONSEPARSER_H

#include "const.h"
#include <QObject>
#include <QString>

enum class HttpResponseParserType
{
    WhApp,
    WhAPPDetail,
    AMap,
    BDMap
};

class HttpResponseHandler;

class HttpResponseParserBase : public QObject
{
    Q_OBJECT

public:
    enum class State
    {
        None,
        Parsing,
        Succeeded,
        Failed
    };

    HttpResponseParserBase(QObject* parent, HttpResponseHandler& handler);
    virtual ~HttpResponseParserBase() = default;

    virtual bool parse(const QString& context, QString& err_msg) = 0;

    HttpResponseHandler& m_handler;

    void setState(State state) { m_state = state; }
    State state() const { return m_state; }

    static void appendTagToString(const QString& tag, QString& str);
    static void extractTagFromString(QString& tag, QString& str);

private:
    State m_state;
};

// Wuhan bendibao
class WhAppHttpResponseParser : public HttpResponseParserBase
{
    Q_OBJECT

public:
    WhAppHttpResponseParser(QObject* parent, HttpResponseHandler& handler);

    virtual bool parse(const QString& context, QString &err_msg) override;
};

// Wuhan bendibao station detail info http parser
class WhAppDetailHttpResponseParser : public HttpResponseParserBase
{
    Q_OBJECT

public:
    WhAppDetailHttpResponseParser(QObject* parent, HttpResponseHandler& handler);

    virtual bool parse(const QString &context, QString &err_msg) override;
};

class BDMapHttpResponseParser : public HttpResponseParserBase
{
    Q_OBJECT

public:
    BDMapHttpResponseParser(QObject* parent, HttpResponseHandler& handler);

    virtual bool parse(const QString &context, QString &err_msg) override;

    // enable other class to call these static function
    static QUrl generateBDMapRequestUrl(const QString& address, const QString& city, const QString& key = BDMAP_KEY);

    void setCount(int count) { m_count = count; }
    int count() const { return m_count; }

private:
    int m_count;
};

class AmapHttpResponseParser : public HttpResponseParserBase
{
    Q_OBJECT

public:
    AmapHttpResponseParser(QObject* parent, HttpResponseHandler& handler);

    virtual bool parse(const QString &context, QString &err_msg) override;
};

// interface class
class HttpResponseParserFactory : public QObject
{
    Q_OBJECT

public:
    HttpResponseParserFactory(QObject* parent, HttpResponseHandler& handler);

    HttpResponseParserBase& getHttpResponseParser(HttpResponseParserType type);

    bool isParseSucceeded();

signals:
    void parseFinished();

private:   
    QVector<HttpResponseParserBase*> m_parsers;

    WhAppHttpResponseParser* m_WhAppParser;
    WhAppDetailHttpResponseParser* m_WhAppDetailParser;
    AmapHttpResponseParser* m_AmapParser;
    BDMapHttpResponseParser* m_BDMapParser;
};

#endif // HTTPRESPONSEPARSER_H
