#include "GeometryWidgetBase.h"

#include "GUIFrame/MainWindow.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/GraphInteractionOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshSizeInfoGenerator.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeGeom.h"

namespace GUI
{
    GeometryWidgetBase::GeometryWidgetBase(Interface::FITKAbsGeoCommand * obj, EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        Core::FITKWidget(parent), _obj(obj), _oper(oper)
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

    void GeometryWidgetBase::createMeshSizeGeo()
    {
        auto meshSizeGen = Interface::FITKMeshGenInterface::getInstance()->getMeshSizeGenerator();
        auto meshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getRegionMeshSizeMgr();
        if (meshSizeGen&&meshSizeManager) {
            auto meshSizeGeo = dynamic_cast<Interface::FITKRegionMeshSizeGeom*>
                (meshSizeGen->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionType::RigonGeom));
            if (meshSizeGeo) {
                meshSizeGeo->setGeomID(_obj->getDataObjectID());
                meshSizeManager->appendDataObj(meshSizeGeo);
            }
        }
    }
}


