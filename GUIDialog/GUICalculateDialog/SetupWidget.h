#ifndef _SetupWidget_H
#define _SetupWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFEnum.hpp"

namespace Ui {
    class SetupWidget;
}

namespace Interface {
    class FITKOFSetUpCase;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUICalculateDialogAPI SetupWidget : public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        SetupWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~SetupWidget();

        void init();

        void updateTableWidget();
    private:
        QString typeToName(Interface::FITKOFSolverEnum::FITKOFSolverType type);
        void initSetupType();
        void initCurrentType();
    private slots:
        void on_radioButton_SteadyState_clicked();

        void on_radioButton_Transient_clicked();

        void on_radioButton_Incompressible_clicked();

        void on_radioButton_Compressible_clicked();

        void on_pushButton_Select_clicked();

    private:
        Interface::FITKOFSetUpCase* _setUpCase = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        Ui::SetupWidget* _ui = nullptr;
        QList<Interface::FITKOFSolverEnum::FITKOFSolverType> _types = {};
    };
}

#endif
