#include "jsonparser.h"
#include "subwaygraph.h"
#include <QFile>
#include <QJsonDocument>

JsonParser::JsonParser()
{

}

JsonParser &JsonParser::getJsonParserInstance()
{
    static JsonParser parser;
    return parser;
}

bool JsonParser::parse(QString &err_msg)
{
    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) != true) {
        err_msg = QString("parse failed as JsonParser can not open file %s").arg(m_fileName);
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close(); // 提前关闭文件

    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(fileData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        err_msg = parseError.errorString();
        return false;
    }

    return true;
}
