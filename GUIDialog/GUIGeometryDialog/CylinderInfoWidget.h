#ifndef _CylinderInfoWidget_H
#define _CylinderInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Ui {
    class CylinderInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelCylinder;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIGeometryDialogAPI CylinderInfoWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        //创建构造函数
        CylinderInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        //编辑构造函数
        CylinderInfoWidget(Interface::FITKAbsGeoModelCylinder* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~CylinderInfoWidget();

        void init();

        void setOriginPoint(double* point);
    private slots:
        ;
        void on_pushButton_OriginPoint_clicked();

        void on_pushButton_Cancel_clicked();

        void on_pushButton_CreateOrEdit_clicked();
    private:
        bool checkValue();
        void setDataToWidget();
        void getDataFormWidget();
    private:
        bool _isCreate = false;
        Interface::FITKAbsGeoModelCylinder* _obj = nullptr;
        Ui::CylinderInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
