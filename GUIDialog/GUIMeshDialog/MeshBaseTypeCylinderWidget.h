#ifndef _MeshBaseTypeCylinderWidget_H
#define _MeshBaseTypeCylinderWidget_H

#include "MeshBaseTypeWidgetBase.h"

namespace Ui {
    class MeshBaseTypeCylinderWidget;
}

namespace GUI
{
    class MeshBaseTypeCylinderWidget :public MeshBaseTypeWidgetBase
    {
        Q_OBJECT;
    public:
        MeshBaseTypeCylinderWidget();
        ~MeshBaseTypeCylinderWidget();

        void init();

        bool checkValue();

        bool setDataToWidget(Interface::FITKAbstractRegionMeshSize* obj);

        bool getDataFromWidget(Interface::FITKAbstractRegionMeshSize* obj);

    private:
        Ui::MeshBaseTypeCylinderWidget* _ui = nullptr;
    };
}

#endif
