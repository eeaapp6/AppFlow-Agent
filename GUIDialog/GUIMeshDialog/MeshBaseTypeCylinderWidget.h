#ifndef _MeshBaseTypeCylinderWidget_H
#define _MeshBaseTypeCylinderWidget_H

#include "MeshBaseTypeWidgetBase.h"

namespace Ui {
    class MeshBaseTypeCylinderWidget;
}

namespace Interface {
    class FITKRegionMeshSizeCylinder;
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
    private slots:
        ;
        void on_pushButton_AutoSize_clicked();

    private:
        Ui::MeshBaseTypeCylinderWidget* _ui = nullptr;
        Interface::FITKRegionMeshSizeCylinder* _graphObj = nullptr;
    };
}

#endif
