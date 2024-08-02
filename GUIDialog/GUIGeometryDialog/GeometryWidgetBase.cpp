#include "GeometryWidgetBase.h"

#include "GUIFrame/MainWindow.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/GraphInteractionOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    GeometryWidgetBase::GeometryWidgetBase(QWidget * parent):
        Core::FITKWidget(parent)
    {
        _mainWin = FITKAPP->getGlobalData()->getMainWindowT<MainWindow>();
    }

    GeometryWidgetBase::~GeometryWidgetBase()
    {

    }

    void GeometryWidgetBase::showEvent(QShowEvent * event)
    {
        transparency();
    }

    void GeometryWidgetBase::closeEvent(QCloseEvent * event)
    {
        transparency(false, false);
    }

    void GeometryWidgetBase::transparency(bool geo, bool mesh)
    {
        //EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        //if (graphOper == nullptr)return;
        //graphOper->setEnableModelTransparent(geo);
        //graphOper->setEnableMeshTransparent(mesh);
        //graphOper->reRender();
    }
}


