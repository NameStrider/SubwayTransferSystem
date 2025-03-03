#ifndef HTTPRESPONSEPARSER_H
#define HTTPRESPONSEPARSER_H

#include <QObject>
#include <QString>

enum class HttpResponseParserType
{
    WhApp,
    AMap
};

class HttpResponseHandler;

class HttpResponseParserBase
{
public:
    explicit HttpResponseParserBase(HttpResponseHandler& handler);
    virtual ~HttpResponseParserBase() = default;

    virtual bool parse(const QString& context, QString& err_msg) = 0;

    HttpResponseHandler& m_handler;
};

class WhAppHttpResponseParser : public HttpResponseParserBase
{
public:
    explicit WhAppHttpResponseParser(HttpResponseHandler& handler);

    virtual bool parse(const QString& context, QString &err_msg) override;
};

class AmapHttpResponseParser : public HttpResponseParserBase
{
public:
    explicit AmapHttpResponseParser(HttpResponseHandler& handler);

    virtual bool parse(const QString &context, QString &err_msg) override;
};

// interface class
class HttpResponseParserFactory : public QObject
{
    Q_OBJECT

public:
    HttpResponseParserFactory(QObject* parent, HttpResponseHandler& handler);

    HttpResponseParserBase& getHttpResponseParser(HttpResponseParserType type);

private:
    WhAppHttpResponseParser m_WhAppParser;
    AmapHttpResponseParser m_AmapParser;
};

#endif // HTTPRESPONSEPARSER_H
