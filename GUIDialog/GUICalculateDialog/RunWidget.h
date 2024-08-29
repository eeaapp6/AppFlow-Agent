#ifndef _RunWidget_H
#define _RunWidget_H

#include <QWidget>
#include "GUICalculateDialogAPI.h"

class QAbstractButton;

namespace Ui {
    class RunWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    enum class RunCPUType {
        Serial,
        Parallel,
    };

    class GUICalculateDialogAPI RunWidget : public QWidget
    {
        Q_OBJECT;
    public:
        RunWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~RunWidget();

        void init();
    private slots:
        ;
        void slotCPUChange(QAbstractButton* button);
        void on_spinBox_NumOfPro_valueChanged(int arg1);

        void on_pushButton_Run_clicked();

    private:
        void initCPU();
        void updateCPU();
    private:
        Ui::RunWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
