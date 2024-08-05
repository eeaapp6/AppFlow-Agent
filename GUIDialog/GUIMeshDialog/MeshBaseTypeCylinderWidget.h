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
        MeshBaseTypeCylinderWidget(QWidget* parent);
        ~MeshBaseTypeCylinderWidget();

        void init();

        bool checkValue();

        bool setDataToWidget(Interface::FITKAbstractRegionMeshSize* obj) override;

        bool getDataFromWidget(Interface::FITKAbstractRegionMeshSize* obj) override;

        void updateGeometryGraph() override;
    private slots:
        ;
        void on_pushButton_AutoSize_clicked();

		void slotSaveValue();
    private:
        Ui::MeshBaseTypeCylinderWidget* _ui = nullptr;
        Interface::FITKRegionMeshSizeCylinder* _graphObj = nullptr;
    };
}

#endif
