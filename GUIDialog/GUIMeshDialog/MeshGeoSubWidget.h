#ifndef _MeshGeoSubWidget_H
#define _MeshGeoSubWidget_H

#include "GUIWidget/GUIWidgetBase.h"

namespace Ui {
    class MeshGeoSubWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKRegionMeshSizeGeom;
}

namespace GUI
{
    class MeshGeoSubWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        MeshGeoSubWidget(int geoID, EventOper::ParaWidgetInterfaceOperator * oper, QWidget* parent = nullptr);
        ~MeshGeoSubWidget();

        void setName(const QString& name);

        int getObjID();
        
        void init();
    private slots:
        void on_spinBox_Min_valueChanged(int arg1);

        void on_spinBox_Max_valueChanged(int arg1);
    private:
        void setDataToWidget();
    private:
        Interface::FITKRegionMeshSizeGeom* _geoMeshSize = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        Ui::MeshGeoSubWidget* _ui = nullptr;
        int _objID = -1;
    };
}

#endif
