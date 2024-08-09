#ifndef _GeometryBoolWidget_H
#define _GeometryBoolWidget_H

#include "GUIGeometryDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"

namespace Ui {
    class GeometryBoolWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    enum class BoolType {
        GeoBoolNone,
        GeoBoolFause,
        GeoBoolCut,
        GeoBoolCommon,
    };

    class GUIGeometryDialogAPI GeometryBoolWidget : public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        GeometryBoolWidget(BoolType type, EventOper::ParaWidgetInterfaceOperator* oper);
        ~GeometryBoolWidget();

        void init();
    private slots:
        ;
        void on_comboBox_Body1_activated(int index);

        void on_comboBox_Body2_activated(int index);

        void on_pushButton_Body1_clicked();

        void on_pushButton_Body2_clicked();

        void on_pushButton_Apply_clicked();
    private:
        bool checkValue();
    private:
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        Ui::GeometryBoolWidget* _ui = nullptr;
        BoolType _type = BoolType::GeoBoolNone;
    };
}

#endif
