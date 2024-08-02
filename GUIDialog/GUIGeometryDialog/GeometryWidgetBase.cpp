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
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->setEnableModelTransparent(true);
        graphOper->setEnableMeshTransparent(true);
        graphOper->reRender();
    }

    void GeometryWidgetBase::closeEvent(QCloseEvent * event)
    {
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->setEnableModelTransparent(false);
        graphOper->setEnableMeshTransparent(false);
        graphOper->reRender();
    }
}


