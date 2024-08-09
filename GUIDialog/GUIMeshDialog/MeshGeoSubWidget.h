#ifndef _MeshGeoSubWidget_H
#define _MeshGeoSubWidget_H

#include "GUIWidget/GUIWidgetBase.h"

namespace Ui {
    class MeshGeoSubWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class MeshGeoSubWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        MeshGeoSubWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget* parent = nullptr);
        ~MeshGeoSubWidget();

        void setName(const QString& name);

        void setObjID(int id);

        int getObjID();
        
        void init();
    private:
        EventOper::ParaWidgetInterfaceOperator* oper = nullptr;
        Ui::MeshGeoSubWidget* _ui = nullptr;
        int _objID = -1;
    };
}

#endif
