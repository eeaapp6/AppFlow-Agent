#ifndef GUIWidgetBase_H
#define GUIWidgetBase_H

#include "GUIWidgetAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace GUI
{
    class MainWindow;

    class GUIWIDGETAPI GUIWidgetBase : public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        GUIWidgetBase(QWidget* parent = nullptr);
        virtual ~GUIWidgetBase(); 

    protected:
        MainWindow* _mainWin = nullptr;
    };
}
#endif