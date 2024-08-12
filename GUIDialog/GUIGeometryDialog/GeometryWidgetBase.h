#ifndef GeometryWidgetBase_H_
#define GeometryWidgetBase_H_

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Interface {
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class MainWindow;

    class GUIGeometryDialogAPI GeometryWidgetBase : public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        GeometryWidgetBase(Interface::FITKAbsGeoCommand* obj, EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        virtual ~GeometryWidgetBase();
    protected:
        void showEvent(QShowEvent *event) override;
        void closeEvent(QCloseEvent* event) override;
        void transparency(bool geo = true, bool mesh = true);

        void createMeshSizeGeo();
    protected:
        MainWindow* _mainWin = nullptr;
        Interface::FITKAbsGeoCommand* _obj = nullptr;
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
