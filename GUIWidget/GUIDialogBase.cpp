#include "GUIDialogBase.h"

#include "GUIFrame/MainWindow.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    GUI::GUIDialogBase::GUIDialogBase(QWidget* parent) :
        Core::FITKDialog(parent)
    {
        _mainWin = dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
    }

    GUI::GUIDialogBase::~GUIDialogBase()
    {

    }
}

