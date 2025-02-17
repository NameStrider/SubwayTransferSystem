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

bool StationNodeParam::isValid() const
{
    if (longitude < MIN_LONGITUDE)
        return false;
    else if (longitude > MAX_LONGITUDE)
        return false;
    else if (latitude < MIN_LATITUDE)
        return false;
    else if (latitude > MAX_LATITUDE)
        return false;

    if (stayTime < MIN_STAY_TIME
        || stayTime > MAX_STAY_TIME)
        return false;

    return true;
}
