#ifndef _MeshBaseTypeBoxWidget_H
#define _MeshBaseTypeBoxWidget_H

#include "MeshBaseTypeWidgetBase.h"

namespace Ui {
    class MeshBaseTypeBoxWidget;
}

namespace GUI
{
    class MeshBaseTypeBoxWidget :public MeshBaseTypeWidgetBase
    {
        Q_OBJECT;
    public:
        MeshBaseTypeBoxWidget();
        ~MeshBaseTypeBoxWidget();

        void init();

        bool checkValue();

        bool setDataToWidget(Interface::FITKAbstractRegionMeshSize* obj);

        bool getDataFromWidget(Interface::FITKAbstractRegionMeshSize* obj);

    private:
        Ui::MeshBaseTypeBoxWidget* _ui = nullptr;
    };
}

#endif
