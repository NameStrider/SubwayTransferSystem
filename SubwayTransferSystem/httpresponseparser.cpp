#include "const.h"
#include "httpresponseparser.h"
#include "networkmanager.h"
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

HttpResponseParserBase::HttpResponseParserBase(QObject* parent, HttpResponseHandler &handler)
    : QObject(parent)
    , m_handler(handler)
    , m_state(State::None)
{}

void HttpResponseParserBase::appendTagToString(const QString &tag, QString &str)
{
    if (tag.isEmpty() || str.isEmpty())
        return;
    str += QString("<%1>").arg(tag);
}

void HttpResponseParserBase::extractTagFromString(QString &tag, QString &str)
{
    const int start = str.lastIndexOf('<');
    const int end = str.lastIndexOf('>');
    if (start != -1 && end != -1 && end > start) {
        tag = str.mid(start + 1, end - start - 1);
        str.truncate(start);
    }
}

WhAppHttpResponseParser::WhAppHttpResponseParser(QObject* parent, HttpResponseHandler &handler)
    : HttpResponseParserBase(parent, handler)
{}

bool WhAppHttpResponseParser::parse(const QString& context, QString &err_msg)
{
    HttpResponseHandler::SubwayLines& subwayLines = m_handler.subwayLines();

    // 匹配每个线路的块
    QRegularExpression lineBlockRegex(
        R"(<div class="line-list">(.*?)<div class="clearfix"></div>\s*</div>\s*</div>\s*</div>)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );

    // 匹配线路编号（从标题中提取）
    QRegularExpression lineNumberRegex(
        R"(武汉地铁(\d+)号线线路图)"
    );

    // 改进后的站点正则：允许class="link"属性位置变化，并确保跨行匹配
    QRegularExpression stationRegex(
        R"(<div class="station">[\s\S]*?<a\s+[^>]*?class="link"[^>]*>([^<]+)</a>)",
        QRegularExpression::CaseInsensitiveOption
    );

    // 遍历所有线路块
    QRegularExpressionMatchIterator blockIter = lineBlockRegex.globalMatch(context);
    while (blockIter.hasNext()) {
        QRegularExpressionMatch blockMatch = blockIter.next();
        QString blockContent = blockMatch.captured(1);

        // 提取线路编号
        QRegularExpressionMatch lineNumberMatch = lineNumberRegex.match(blockContent);
        if (!lineNumberMatch.hasMatch()) continue;
        int lineNumber = lineNumberMatch.captured(1).toInt();

        // 提取站点名称
        QVector<QString> stations;
        QRegularExpressionMatchIterator stationIter = stationRegex.globalMatch(blockContent);
        while (stationIter.hasNext()) {
            QRegularExpressionMatch stationMatch = stationIter.next();
            QString stationName = stationMatch.captured(1).trimmed();
            stations.append(stationName);
        }

        // 存入结果
        subwayLines.insert(lineNumber, stations);
    }

    // extract deatil url
    QSet<QString> urlStrings;
    // 匹配两种格式的链接，并排除换乘线路的链接（xl_开头的URL）
    QRegularExpression regex(
        R"REGEX(<div\s+class="station"[^>]*>.*?<a\s+(?:[^>]*?\s+)?href="(/ditie/zd_[^"]+\.shtml)"[^>]*>)REGEX",
        QRegularExpression::CaseInsensitiveOption |
        QRegularExpression::DotMatchesEverythingOption
    );

    QRegularExpressionMatchIterator it = regex.globalMatch(context);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString url = match.captured(1);
        // 排除换乘线路的干扰（如 /ditie/xl_9.shtml）
        if (url.startsWith("/ditie/zd_")) {
            urlStrings.insert(url);
        }
    }

#if 0
    // request bendibao detail page
    QSet<QUrl> urls;
    for (const QString& urlString : urlStrings) {
        QString completeUrlString = QString(WH_APP_DOMAIN_NAME) + urlString;
        urls.insert(completeUrlString);
    }
    // unreasonable, HttpRequestHandler could emit sigal
    emit m_handler.httpRequestPending(urls);
#endif

    // write StationInfos
    HttpResponseHandler::StationInfos& stationInfos = m_handler.stationInfos();
    for (auto line = subwayLines.begin(); line != subwayLines.end(); ++line) {
        for (const auto& station : line.value()) {
            if (stationInfos.find(station) == stationInfos.end()) {
                StationInfo stationInfo;
                stationInfo.basicInfo.name = station;
                stationInfo.basicInfo.stayTime = DEFAULT_STAY_TIME;
                stationInfo.basicInfo.belongingLines.insert(line.key());
                stationInfos.insert(station, stationInfo);

                // qDebug() << stationInfo.basicInfo.name << stationInfo.basicInfo.belongingLines;
            }
            else {
                stationInfos[station].basicInfo.belongingLines.insert(line.key());
            }
        }
    }

    // request baidu map
    QSet<QUrl> pendingUrls;
    QSet<QString> stations;
    for (auto line = subwayLines.begin(); line != subwayLines.end(); ++line) {
        for (const auto& station : line.value()) {
            if (stations.find(station) != stations.end()) {
                continue;
            }
            stations.insert(station);
            // use complete address to precisely request
            // should decouple
            QString completeAddress = QString("武汉市") + station + "地铁站";
            const QUrl& url = BDMapHttpResponseParser::generateBDMapRequestUrl(completeAddress);
            pendingUrls.insert(url);
        }
    }

    HttpResponseParserBase& BDMapParser = m_handler.getHttpResponseParserFactory().getHttpResponseParser(HttpResponseParserType::BDMap);
    static_cast<BDMapHttpResponseParser&>(BDMapParser).setCount(pendingUrls.size());
    m_handler.schedulePendingHttpRequest(pendingUrls, BDMAP_REQUEST_INTERVAL);

    return true;
}

WhAppDetailHttpResponseParser::WhAppDetailHttpResponseParser(QObject* parent, HttpResponseHandler &handler)
    : HttpResponseParserBase(parent, handler)
{}

bool WhAppDetailHttpResponseParser::parse(const QString &context, QString &err_msg)
{
    // 第一步：提取目标区域的HTML内容（位于<h2>站点换乘信息</h2>后的<div class="summary"><p>内）
    QRegularExpression contentRegex(
        R"(<h2>站点换乘信息<\/h2>\s*<div class="summary"><p>(.*?)<\/p><\/div>)",
        QRegularExpression::DotMatchesEverythingOption
    );
    QRegularExpressionMatch contentMatch = contentRegex.match(context);
    if (!contentMatch.hasMatch()) {
        // 未找到目标区域则退出
        return false;
    }
    QString targetContent = contentMatch.captured(1);

    // 第二步：在目标区域内提取站点名
    QRegularExpression stationRegex(R"(<a\s+href="/ditie/zd_\w+\.shtml"[^>]*>([^<]+)</a>)");
    QRegularExpressionMatchIterator stationIt = stationRegex.globalMatch(targetContent);
    QStringList stationNames;
    while (stationIt.hasNext()) {
        QString name = stationIt.next().captured(1).trimmed();
        if (!name.isEmpty() && !stationNames.contains(name)) {
            stationNames.append(name);
        }
    }

    // 第三步：在目标区域内提取线路号（仅匹配"地铁X号线"格式）
    QRegularExpression lineRegex(R"(<a\s+href="/ditie/xl_\d+\.shtml"[^>]*>地铁(\d+)号线</a>)");
    QRegularExpressionMatchIterator lineIt = lineRegex.globalMatch(targetContent);
    QStringList lineNumbers;
    while (lineIt.hasNext()) {
        lineNumbers.append(lineIt.next().captured(1));
    }

    // 输出结果
    if (!stationNames.isEmpty() && !lineNumbers.isEmpty()) {
        qDebug() << "站点名：" << stationNames.join(", ")
                 << "，所属线路：" << lineNumbers.join(", ");
    }

    return true;
}

BDMapHttpResponseParser::BDMapHttpResponseParser(QObject* parent, HttpResponseHandler &handler)
    : HttpResponseParserBase(parent, handler)
    , m_count(-1)
{}

bool BDMapHttpResponseParser::parse(const QString &context, QString &err_msg)
{
    if (context.isEmpty()) {
        err_msg = "http response parse failed as context is empty";
        return false;
    }

    // tail of context includes tag "<address>", extract param first and then parse
    QString tag;
    QString& str = const_cast<QString&>(context);
    HttpResponseParserBase::extractTagFromString(tag, str);    

    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(context.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        err_msg = "http response parse failed as JSON parse failed";
        return false;
    }

    QJsonObject root = jsonDocument.object();
    QJsonObject result = root["result"].toObject();
    QJsonObject location = result["location"].toObject();
    double longitude = location["lng"].toDouble();
    double latitude = location["lat"].toDouble();

    QString stationName = tag.mid(3, tag.length() - 6);
    HttpResponseHandler::StationInfos& stationInfos = m_handler.stationInfos();
    if (stationInfos.find(stationName) != stationInfos.end()) {
        StationInfo& stationInfo = stationInfos[stationName];
        stationInfo.basicInfo.longitude = longitude;
        stationInfo.basicInfo.latitude = latitude;
    }  

    if (m_count < 0) {
        qDebug() << "m_count error";
    }
    m_count--;
    if (m_count == 0) {
        // not reasonable, to be improved
        emit m_handler.getHttpResponseParserFactory().parseFinished();
    }

    qDebug() << tag << longitude << latitude << m_count;

    return true;
}

QUrl BDMapHttpResponseParser::generateBDMapRequestUrl(const QString &address, const QString &key)
{
    QString urlTemplateStr = BDMAP_REQUEST_URL_TEMPLATE;
    return QUrl(urlTemplateStr.arg(address).arg(key));
}

AmapHttpResponseParser::AmapHttpResponseParser(QObject* parent, HttpResponseHandler &handler)
    : HttpResponseParserBase(parent, handler)
{}

bool AmapHttpResponseParser::parse(const QString &context, QString &err_msg)
{

}

HttpResponseParserFactory::HttpResponseParserFactory(QObject *parent, HttpResponseHandler &handler)
    : QObject(parent)
    , m_WhAppParser(new WhAppHttpResponseParser(parent, handler))
    , m_WhAppDetailParser(new WhAppDetailHttpResponseParser(parent, handler))
    , m_AmapParser(new AmapHttpResponseParser(parent, handler))
    , m_BDMapParser(new BDMapHttpResponseParser(parent, handler))
{
    // use m_WhAppParser and m_BDMapParser only now
    m_parsers.push_back(m_WhAppParser);
    m_parsers.push_back(m_BDMapParser);
}

HttpResponseParserBase &HttpResponseParserFactory::getHttpResponseParser(HttpResponseParserType type)
{
    switch (type) {
    case HttpResponseParserType::WhApp:
        return *m_WhAppParser;
    case HttpResponseParserType::AMap:
        return *m_AmapParser;
    case HttpResponseParserType::WhAPPDetail:
        return *m_WhAppDetailParser;
    case HttpResponseParserType::BDMap:
        return *m_BDMapParser;
    default:
        return *m_WhAppParser;
    }
}

bool HttpResponseParserFactory::isParseSucceeded()
{
    bool flag = true;
    for (auto parser : m_parsers) {
        if (parser->state() == HttpResponseParserBase::State::Failed)
            flag = false;
    }
    for (auto parser : m_parsers) {
        parser->setState(HttpResponseParserBase::State::None);
    }
    return flag;
}
