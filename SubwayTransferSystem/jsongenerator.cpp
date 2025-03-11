#include "jsongenerator.h"

JsonGenerator &JsonGenerator::getJsonGenerator()
{
    static JsonGenerator generator;
    return generator;
}

bool JsonGenerator::generate(const HttpResponseHandler::SubwayLines &subwayLines, const HttpResponseHandler::StationInfos &stationInfos)
{

}

JsonGenerator::JsonGenerator()
{

}
