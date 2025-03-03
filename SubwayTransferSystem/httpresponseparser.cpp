#include "httpresponseparser.h"
#include "networkmanager.h"
#include <QRegularExpression>

HttpResponseParserBase::HttpResponseParserBase(HttpResponseHandler &handler)
    : m_handler(handler)
{}

WhAppHttpResponseParser::WhAppHttpResponseParser(HttpResponseHandler &handler)
    : HttpResponseParserBase(handler)
{}

bool WhAppHttpResponseParser::parse(const QString& context, QString &err_msg)
{
    QHash<int, QVector<QPair<QString, QString>>> subwayLines;

    // 修复线路块正则表达式（转义闭合标签）
    QRegularExpression lineBlockRegex(
        R"(<div class="line-list">(.*?)<div class="clearfix"></div>\s*<\/div>\s*<\/div>\s*<\/div>)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );

    // 线路编号正则保持不变
    QRegularExpression lineNumberRegex(R"(武汉地铁(\d+)号线线路图)");

    // 修复站点正则：改用普通字符串并转义特殊字符
    QRegularExpression stationRegex(
        "<div class=\"station\">.*?<a\\s+[^>]*?href=\"([^\"]*)\"[^>]*?class=\"link\"[^>]*>([^<]+)</a>",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatchIterator blockIter = lineBlockRegex.globalMatch(context);
    while (blockIter.hasNext()) {
        QRegularExpressionMatch blockMatch = blockIter.next();
        QString blockContent = blockMatch.captured(1);

        QRegularExpressionMatch lineNumberMatch = lineNumberRegex.match(blockContent);
        if (!lineNumberMatch.hasMatch()) continue;
        int lineNumber = lineNumberMatch.captured(1).toInt();

        QVector<QPair<QString, QString>> stations;
        QRegularExpressionMatchIterator stationIter = stationRegex.globalMatch(blockContent);
        while (stationIter.hasNext()) {
            QRegularExpressionMatch stationMatch = stationIter.next();
            QString url = stationMatch.captured(1);
            QString name = stationMatch.captured(2).trimmed();
            stations.append(qMakePair(name, url));
        }

        if (!stations.isEmpty()) {
            subwayLines.insert(lineNumber, stations);
        }
    }

    for (auto line = subwayLines.begin(); line != subwayLines.end(); ++line) {
        QStringList context;
        context << QString::number(line.key());
        for (const auto& stations : line.value()) {
            context << stations.first << stations.second;
        }
        qDebug() << context.join(" ");
    }

    return true;
}

AmapHttpResponseParser::AmapHttpResponseParser(HttpResponseHandler &handler)
    : HttpResponseParserBase(handler)
{}

bool AmapHttpResponseParser::parse(const QString &context, QString &err_msg)
{

}

HttpResponseParserFactory::HttpResponseParserFactory(QObject *parent, HttpResponseHandler &handler)
    : QObject(parent)
    , m_WhAppParser(handler)
    , m_AmapParser(handler)
{}

HttpResponseParserBase &HttpResponseParserFactory::getHttpResponseParser(HttpResponseParserType type)
{
    switch (type) {
    case HttpResponseParserType::WhApp:
        return m_WhAppParser;
        break;
    case HttpResponseParserType::AMap:
        return m_AmapParser;
        break;
    default:
        break;
    }
}
