#ifndef SUBWAYGRAPHVIEW_H
#define SUBWAYGRAPHVIEW_H

#include "subwaygraphscene.h"
#include <QGraphicsView>

class SubwayGraphView : public QGraphicsView
{
    Q_OBJECT

public:
    SubwayGraphView(QWidget* parent = nullptr);

    SubwayGraphScene* m_scene;
};

#endif // SUBWAYGRAPHVIEW_H
