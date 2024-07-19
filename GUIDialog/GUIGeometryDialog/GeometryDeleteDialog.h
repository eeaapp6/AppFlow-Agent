#ifndef GeometryDeleteDialog_H
#define GeometryDeleteDialog_H

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Ui {
    class GeometryDeleteDialog;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKAbsGeoCommand;
}

namespace GUI 
{    
    class GUIGeometryDialogAPI GeometryDeleteDialog : public Core::FITKDialog
    {
        Q_OBJECT;
    public:
        GeometryDeleteDialog(Interface::FITKAbsGeoCommand* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~GeometryDeleteDialog();
    private slots:
        ;
        /**
         * @brief Ok点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void on_pushButton_OK_clicked();
        /**
         * @brief 取消点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void on_pushButton_Cancel_clicked();
    protected:
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        Ui::GeometryDeleteDialog* _ui = nullptr;
        Interface::FITKAbsGeoCommand* _obj = nullptr;
    };
}

#endif
