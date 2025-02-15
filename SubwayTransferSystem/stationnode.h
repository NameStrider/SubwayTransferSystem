#ifndef STATIONNODE_H
#define STATIONNODE_H

#include "stationnodeparam.h"

class StationNode
{
public:
    StationNode(const StationNodeParam& _param);
    // name 作为唯一标识 key 不能赋值或者复制
    StationNode(const StationNode& other) = delete;
    StationNode& operator=(const StationNode& other) = delete;

    StationNodeParam param;
};

#endif // STATIONNODE_H
