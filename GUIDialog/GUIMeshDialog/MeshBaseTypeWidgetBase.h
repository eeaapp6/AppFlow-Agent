#ifndef _MeshBaseTypeWidgetBase_H
#define _MeshBaseTypeWidgetBase_H

#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Interface {
    class FITKAbstractRegionMeshSize;
}

namespace GUI 
{
    class MeshBaseTypeWidgetBase :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        MeshBaseTypeWidgetBase();
        ~MeshBaseTypeWidgetBase();

        virtual bool checkValue() = 0;

        virtual bool setDataToWidget(Interface::FITKAbstractRegionMeshSize* obj) = 0;

        virtual bool getDataFromWidget(Interface::FITKAbstractRegionMeshSize* obj) = 0;
    };
}

#endif
