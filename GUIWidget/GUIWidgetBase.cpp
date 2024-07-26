#include "GUIWidgetBase.h"

#include "GUIFrame/MainWindow.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    GUI::GUIWidgetBase::GUIWidgetBase(QWidget* parent):
        Core::FITKWidget(parent)
    {
        _mainWin = dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
    }

    GUI::GUIWidgetBase::~GUIWidgetBase()
    {

    }
}

