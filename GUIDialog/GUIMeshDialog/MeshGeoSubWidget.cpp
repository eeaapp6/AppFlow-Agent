#include "MeshGeoSubWidget.h"
#include "ui_MeshGeoSubWidget.h"

#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeGeom.h"

namespace GUI
{
    MeshGeoSubWidget::MeshGeoSubWidget(int geoID, EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUIWidgetBase(parent), _objID(geoID), _oper(oper)
    {
        _ui = new Ui::MeshGeoSubWidget();
        _ui->setupUi(this);

       Interface::FITKRegionMeshSizeManager* meshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getRegionMeshSizeMgr();
       if (meshSizeManager == nullptr)return;
       auto geoMeshSizeList = meshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RigonGeom);
       for (int i = 0; i < geoMeshSizeList.size(); i++) {
           Interface::FITKRegionMeshSizeGeom* geoMeshSize = dynamic_cast<Interface::FITKRegionMeshSizeGeom*>(geoMeshSizeList[i]);
           if (geoMeshSize == nullptr)continue;
           if (geoMeshSize->getGeomID() == _objID) {
               _geoMeshSize = geoMeshSize;
               break;
           }
       }

       setDataToWidget();
    }


    MeshGeoSubWidget::~MeshGeoSubWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void MeshGeoSubWidget::setName(const QString & name)
    {
        _ui->groupBox_Geo->setTitle(name);
    }

    int MeshGeoSubWidget::getObjID()
    {
        return _objID;
    }

    void MeshGeoSubWidget::init()
    {

    }

    void MeshGeoSubWidget::on_spinBox_Min_valueChanged(int arg1)
    {
        Q_UNUSED(arg1);

        if (_geoMeshSize == nullptr)return;
        int value = _ui->spinBox_Min->value();
        _geoMeshSize->setMinSize(value);
    }

    void MeshGeoSubWidget::on_spinBox_Max_valueChanged(int arg1)
    {
        Q_UNUSED(arg1);

        if (_geoMeshSize == nullptr)return;
        int value = _ui->spinBox_Max->value();
        _geoMeshSize->setMaxSize(value);
    }

    void MeshGeoSubWidget::setDataToWidget()
    {
        if (_geoMeshSize == nullptr)return;
        _ui->spinBox_Max->setValue(_geoMeshSize->getMaxSize());
        _ui->spinBox_Min->setValue(_geoMeshSize->getMinSize());
    }
}

