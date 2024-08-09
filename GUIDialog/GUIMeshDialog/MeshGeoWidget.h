#ifndef _MeshGeoWidget_H
#define _MeshGeoWidget_H

#include "GUIMeshDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"

class QVBoxLayout;

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIMeshDialogAPI MeshGeoWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        MeshGeoWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget* parent = nullptr);
        ~MeshGeoWidget();

        void init();

        void updateSubWidget();
    private:
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        QVBoxLayout* _subWidgetLayout = nullptr;
    };
}

#endif
