#ifndef STATIONNODEPARAM_H
#define STATIONNODEPARAM_H

#include "const.h"
#include <QSet>
#include <QString>

// 参数搬运工
class StationNodeParam
{
public:
    StationNodeParam(const QString& _name, double _longitude
                     , double _latitude, int _stayTime, const QSet<int>& _lines);
    StationNodeParam(const StationNodeParam& other);
    StationNodeParam& operator=(const StationNodeParam& other);

    bool isValid() const;

    QString name;
    double longitude;
    double latitude;
    int stayTime;
    QSet<int> belongingLines;
};

#endif // STATIONNODEPARAM_H
