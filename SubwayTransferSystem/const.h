#ifndef CONST_H
#define CONST_H

#include <QString>

constexpr double MIN_LONGITUDE = -180.0000;
constexpr double MAX_LONGITUDE = 180.0000;
constexpr double MIN_LATITUDE = -90.0000;
constexpr double MAX_LATITUDE = 90.0000;
constexpr int    MIN_STAY_TIME = 1;
constexpr int    MAX_STAY_TIME = 6;
constexpr int    MAX_STATION_NODE = 500;
constexpr int    MIN_LINE_ID = 1;
constexpr int    MAX_LINE_ID = 30;
constexpr int    MIN_DISTANCE = 1;
constexpr int    MAX_DISTANCE = 10;
constexpr int    MIN_STATION_IN_LINE = 2;

constexpr char DEFAULT_WUHAN_METRO_REQUEST_URL[] = "http://wh.bendibao.com/ditie/linemap.shtml";
constexpr char AMAP_KEY[] = "c079b555e5bce39d8b5192fdc3ec5ec3";

#endif // CONST_H
