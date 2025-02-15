#include "stationnodeparam.h"



StationNodeParam::StationNodeParam(const QString &_name, double _longitude, double _latitude
                                   , int _stayTime, const QSet<int> &_lines)
    : name(_name)
    , longitude(_longitude)
    , latitude(_latitude)
    , stayTime(_stayTime)
    , belongingLines(_lines)
{}

StationNodeParam::StationNodeParam(const StationNodeParam &other)
    : name(other.name)
    , longitude(other.longitude)
    , latitude(other.latitude)
    , stayTime(other.stayTime)
    , belongingLines(other.belongingLines)
{}

bool StationNodeParam::isValid(const StationNodeParam &param)
{
    if (param.longitude < MIN_LONGITUDE
        || param.longitude > MAX_LONGITUDE
        || param.latitude < MIN_LATITUDE
        || param.latitude > MAX_LATITUDE)
        return false;

    if (param.stayTime < MIN_STAY_TIME
        || param.stayTime > MAX_STAY_TIME)
        return false;

    return true;
}
