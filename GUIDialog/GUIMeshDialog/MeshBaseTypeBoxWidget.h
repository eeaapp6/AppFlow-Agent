#ifndef _MeshBaseTypeBoxWidget_H
#define _MeshBaseTypeBoxWidget_H

#include "MeshBaseTypeWidgetBase.h"

namespace Ui {
    class MeshBaseTypeBoxWidget;
}

namespace Interface {
    class FITKRegionMeshSizeBox;
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
    private slots:
        ;
        void on_pushButton_AutoSize_clicked();

    private:
        Ui::MeshBaseTypeBoxWidget* _ui = nullptr;
        Interface::FITKRegionMeshSizeBox* _graphObj = nullptr;
    };
}

#endif
