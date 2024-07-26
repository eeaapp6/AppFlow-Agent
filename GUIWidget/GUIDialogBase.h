#ifndef _GUIDialogBase_H
#define _GUIDialogBase_H

#include "GUIWidgetAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace GUI
{
    class MainWindow;

    class GUIWIDGETAPI GUIDialogBase : public Core::FITKDialog
    {
        Q_OBJECT;
    public:
        GUIDialogBase(QWidget* parent = nullptr);
        virtual ~GUIDialogBase();

    protected:
        MainWindow* _mainWin = nullptr;
    };
}


#endif