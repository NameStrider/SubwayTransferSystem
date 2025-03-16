#ifndef CONST_H
#define CONST_H

// graph
constexpr double MIN_LONGITUDE = -180.0000;
constexpr double MAX_LONGITUDE = 180.0000;
constexpr double MIN_LATITUDE = -90.0000;
constexpr double MAX_LATITUDE = 90.0000;
constexpr int    MIN_STAY_TIME = 1;
constexpr int    MAX_STAY_TIME = 9;
constexpr int    DEFAULT_STAY_TIME = 2;
constexpr int    MAX_STATION_NODE = 500;
constexpr int    MIN_LINE_ID = 1;
constexpr int    MAX_LINE_ID = 30;
constexpr int    MIN_DISTANCE = 1;
constexpr int    MAX_DISTANCE = __INT_MAX__;
constexpr int    MIN_STATION_IN_LINE = 2;

// network
constexpr char AMAP_KEY[] = "c079b555e5bce39d8b5192fdc3ec5ec3";
constexpr char BDMAP_KEY[] = "gCQdJSCXaP69aLsjeAUTTcq7Vg7Fpoxu";
constexpr char BDMAP_DOMAIN_NAME[] = "https://api.map.baidu.com";
constexpr char WH_APP_DOMAIN_NAME[] = "https://wh.bendibao.com";
constexpr char DEFAULT_WUHAN_METRO_REQUEST_URL[] = "http://wh.bendibao.com/ditie/linemap.shtml";
constexpr char BDMAP_REQUEST_URL_TEMPLATE[] = "https://api.map.baidu.com/geocoding/v3/?address=%1&city=%2&output=json&ak=%3";

// timer
const int BDMAP_REQUEST_INTERVAL = 1000 / 3;

#endif // CONST_H
