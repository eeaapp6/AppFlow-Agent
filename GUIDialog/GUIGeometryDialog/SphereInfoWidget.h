#ifndef _SphereInfoWidget_H
#define _SphereInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Ui {
    class SphereInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelSphere;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIGeometryDialogAPI SphereInfoWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        //创建构造函数
        SphereInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        //编辑构造函数
        SphereInfoWidget(Interface::FITKAbsGeoModelSphere* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~SphereInfoWidget();

        void init();

        void setCenterPoint(double* point);
    private slots:
        ;
        void on_pushButton_CenterPoint_clicked();

        void on_pushButton_Cancel_clicked();

        void on_pushButton_CreateOrEdit_clicked();

    private:
        bool checkValue();
        void setDataToWidget();
        void getDataFormWidget();
    private:
        bool _isCreate = false;
        Interface::FITKAbsGeoModelSphere* _obj = nullptr;
        Ui::SphereInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
