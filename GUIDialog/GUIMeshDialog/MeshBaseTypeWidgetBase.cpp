#include "MeshBaseTypeWidgetBase.h"
#include "MeshBaseWidget.h"

#include "GUIFrame/MainWindow.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    MeshBaseTypeWidgetBase::MeshBaseTypeWidgetBase(QWidget* parent) :
        Core::FITKWidget(parent)
    {
		_meshBaseWidget = dynamic_cast<MeshBaseWidget*>(parent);
		_mainWin = dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
    }

    MeshBaseTypeWidgetBase::~MeshBaseTypeWidgetBase()
    {

    }
}
