#include "GeometryDeleteDialog.h"
#include "ui_GeometryDeleteDialog.h"
#include "CudeInfoWidget.h"
#include "CylinderInfoWidget.h"
#include "SphereInfoWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
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

        //判断删除的数据是否是当前界面
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        auto type = _obj->getGeometryCommandType();
        QWidget* widget = nullptr;
        int objID = -1;
        switch (type){
        case Interface::FITKGeoEnum::FGTBox: {
            GUI::CudeInfoWidget* cudeWidget = dynamic_cast<GUI::CudeInfoWidget*>(propertyWidget->getCurrentWidget());
            if (cudeWidget == nullptr)return;
            if (cudeWidget->getCurrentGeoCommand()) {
                objID = cudeWidget->getCurrentGeoCommand()->getDataObjectID();
                widget = cudeWidget;
            }
            break;
        }
        case Interface::FITKGeoEnum::FGTCylinder: {
            GUI::CylinderInfoWidget* cudeWidget = dynamic_cast<GUI::CylinderInfoWidget*>(propertyWidget->getCurrentWidget());
            if (cudeWidget == nullptr)return;
            if (cudeWidget->getCurrentGeoCommand()) {
                objID = cudeWidget->getCurrentGeoCommand()->getDataObjectID();
                widget = cudeWidget;
            }
            break;
        }
        case Interface::FITKGeoEnum::FGTSphere: {
            GUI::SphereInfoWidget* cudeWidget = dynamic_cast<GUI::SphereInfoWidget*>(propertyWidget->getCurrentWidget());
            if (cudeWidget == nullptr)return;
            if (cudeWidget->getCurrentGeoCommand()) {
                objID = cudeWidget->getCurrentGeoCommand()->getDataObjectID();
                widget = cudeWidget;
            }
            break;
        }
        }

        //如果删除的数据是当前界面,删除当前界面
        if (objID == _obj->getDataObjectID() && widget) {
            propertyWidget->init();
        }

        geometryData->removeDataByID(_obj->getDataObjectID());
        _oper->execProfession();
        this->accept();
    }

    void GeometryDeleteDialog::on_pushButton_Cancel_clicked()
    {
        this->reject();
    }
}
