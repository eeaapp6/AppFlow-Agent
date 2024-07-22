#include "MeshBaseTypeWidgetBase.h"

#include "GUIFrame/MainWindow.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    MeshBaseTypeWidgetBase::MeshBaseTypeWidgetBase() :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow()))
    {

    }

    MeshBaseTypeWidgetBase::~MeshBaseTypeWidgetBase()
    {

    }
}
