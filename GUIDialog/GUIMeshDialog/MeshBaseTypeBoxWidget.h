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

        bool setDataToWidget(Interface::FITKAbstractRegionMeshSize* obj) override;

        bool getDataFromWidget(Interface::FITKAbstractRegionMeshSize* obj) override;

        void updateGeometryGraph() override;
    private slots:
        ;
        void on_pushButton_AutoSize_clicked();

        void on_lineEdit_BasePoint1_textEdited(const QString &value);

        void on_lineEdit_BasePoint2_textEdited(const QString &value);
        
        void on_lineEdit_BasePoint3_textEdited(const QString &value);
        
        void on_lineEdit_Dimensions1_textEdited(const QString &value);
        
        void on_lineEdit_Dimensions2_textEdited(const QString &value);
        
        void on_lineEdit_Dimensions3_textEdited(const QString &value);
    private:
        Ui::MeshBaseTypeBoxWidget* _ui = nullptr;
        Interface::FITKRegionMeshSizeBox* _graphObj = nullptr;
    };
}

#endif
