#ifndef _GUIWidgetRadioGroup_H
#define _GUIWidgetRadioGroup_H

#include "GUICalculateDialogAPI.h"
#include <QWidget>

namespace Ui {
    class GUIWidgetRadioGroup;
}

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataRadioGroup;
}

namespace GUI
{
    class GUICalculateDialogAPI GUIWidgetRadioGroup : public QWidget
    {
        Q_OBJECT;
    public:
        GUIWidgetRadioGroup(Interface::FITKFlowDataBase* dataBase, QWidget* parent = nullptr);
        ~GUIWidgetRadioGroup();

        void init();
    private:
        void initRadioData();
        void initSubData();
    private:
        Ui::GUIWidgetRadioGroup* _ui = nullptr;
        Interface::FITKFlowDataRadioGroup* _data = nullptr;
    };
}

#endif
