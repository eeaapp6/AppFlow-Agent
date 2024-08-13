#ifndef _GUIWidgetInt_H
#define _GUIWidgetInt_H

#include "GUICalculateDialogAPI.h"
#include <QSpinBox>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataInt;
}

namespace GUI
{
    class GUICalculateDialogAPI GUIWidgetInt :public QSpinBox
    {
        Q_OBJECT;
    public:
        GUIWidgetInt(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        ~GUIWidgetInt();

        void init();
    private slots:
        ;
        void slotDataChange();
    protected:
        Interface::FITKFlowDataInt* _value = nullptr;
    };
}

#endif
