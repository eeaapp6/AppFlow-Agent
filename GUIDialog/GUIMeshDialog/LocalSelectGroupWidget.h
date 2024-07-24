#ifndef _LocalSelectGroupWidget_H
#define _LocalSelectGroupWidget_H

#include "GUIWidget/GUIWidgetBase.h"
#include "GUIMeshDialogAPI.h"

namespace Ui {
    class LocalSelectGroupWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIMeshDialogAPI LocalSelectGroupWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        LocalSelectGroupWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        ~LocalSelectGroupWidget();

        void init();

    private slots:
        ;
        void on_pushButton_Cancel_clicked();

        void on_pushButton_OK_clicked();

    private:
        Ui::LocalSelectGroupWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };

}

#endif
