#include "GeometryDeleteDialog.h"
#include "ui_GeometryDeleteDialog.h"

#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"

namespace GUI
{
    GeometryDeleteDialog::GeometryDeleteDialog(Interface::FITKAbsGeoCommand * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        _oper(oper), _obj(obj)
    {
        _ui = new Ui::GeometryDeleteDialog();
        _ui->setupUi(this);

        if (_obj) {
            QString text = tr("Delete %1").arg(_obj->getDataObjectName());
            _ui->label_Name->setText(text);
        }
    }

    GeometryDeleteDialog::~GeometryDeleteDialog()
    {
        if (_ui)delete _ui;
    }

    void GeometryDeleteDialog::on_pushButton_OK_clicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;
        geometryData->removeDataByID(_obj->getDataObjectID());
        _oper->execProfession();
        this->accept();
    }

    void GeometryDeleteDialog::on_pushButton_Cancel_clicked()
    {
        this->reject();
    }
}
