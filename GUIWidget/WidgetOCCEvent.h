#ifndef _WidgetOCCEvent_H
#define _WidgetOCCEvent_H

#include "GUIWidgetAPI.h"
#include <QObject>

namespace GraphData {
    class PickedData;
}

namespace GUI
{
    class GUIWIDGETAPI WidgetOCCEvent : public QObject
    {
        Q_OBJECT;
    public:
        WidgetOCCEvent();
        ~WidgetOCCEvent();

        static bool getPoint(GraphData::PickedData* data, double* point);

        static QList<int> getFaces(QList<GraphData::PickedData*> data, bool isOk = true);
    };
}

#endif
