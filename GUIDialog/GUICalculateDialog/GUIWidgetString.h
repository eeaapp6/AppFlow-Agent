#ifndef _GUIWidgetString_H
#define _GUIWidgetString_H

#include "GUICalculateDialogAPI.h"
#include <QLineEdit>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataString;
}

namespace GUI
{
    class GUICalculateDialogAPI GUIWidgetString :public QLineEdit
    {
        Q_OBJECT;
    public:
        GUIWidgetString(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        ~GUIWidgetString();

        void init();
    private slots:
        ;
        void slotDataChange();
    protected:
        Interface::FITKFlowDataString* _value = nullptr;
    };
}

#endif
