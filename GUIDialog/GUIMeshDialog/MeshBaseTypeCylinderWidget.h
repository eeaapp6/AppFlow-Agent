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

        bool setDataToWidget(Interface::FITKAbstractRegionMeshSize* obj) override;

        bool getDataFromWidget(Interface::FITKAbstractRegionMeshSize* obj) override;

        void updateGeometryGraph() override;
    private slots:
        ;
        void on_pushButton_AutoSize_clicked();

        void on_lineEdit_OriginPoint1_textEdited(const QString &value);

        void on_lineEdit_OriginPoint2_textEdited(const QString &value);

        void on_lineEdit_OriginPoint3_textEdited(const QString &value);

        void on_lineEdit_AxisPoint1_textEdited(const QString &value);

        void on_lineEdit_AxisPoint2_textEdited(const QString &value);

        void on_lineEdit_AxisPoint3_textEdited(const QString &value);

        void on_lineEdit_Length_textEdited(const QString &value);

        void on_lineEdit_Radius_textEdited(const QString &value);

    private:
        Ui::MeshBaseTypeCylinderWidget* _ui = nullptr;
        Interface::FITKRegionMeshSizeCylinder* _graphObj = nullptr;
    };
}

#endif
