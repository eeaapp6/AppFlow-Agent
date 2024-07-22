#include "MeshBaseTypeCylinderWidget.h"
#include "ui_MeshBaseTypeCylinderWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeBox.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeCylinder.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeSphere.h"

namespace GUI
{
    MeshBaseTypeCylinderWidget::MeshBaseTypeCylinderWidget()
    {
        _ui = new Ui::MeshBaseTypeCylinderWidget();
        _ui->setupUi(this);
    }

    MeshBaseTypeCylinderWidget::~MeshBaseTypeCylinderWidget()
    {
        if (_ui)delete _ui;
    }

    bool MeshBaseTypeCylinderWidget::checkValue()
    {
        return true;
    }

    bool MeshBaseTypeCylinderWidget::setDataToWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        return true;
    }

    bool MeshBaseTypeCylinderWidget::getDataFromWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        return true;
    }
}
