#include "CylinderInfoWidget.h"
#include "ui_CylinderInfoWidget.h"

#include "GUIFrame/MainWindow.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelCylinder.h"

#include <QMessageBox>

namespace GUI {

    CylinderInfoWidget::CylinderInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Create"));
    }

    CylinderInfoWidget::CylinderInfoWidget(Interface::FITKAbsGeoModelCylinder * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(false), _obj(obj), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Edit"));
        _ui->lineEdit_Name->setEnabled(false);
    }

    CylinderInfoWidget::~CylinderInfoWidget()
    {
        if (_ui)delete _ui;
    }

    void CylinderInfoWidget::init()
    {
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        _ui = new Ui::CylinderInfoWidget();
        _ui->setupUi(this);

        QString name = "";
        if (_isCreate) {
            name = QString(tr("Cylinder-%1").arg(geometryData->getDataCount() + 1));
            _ui->lineEdit_Name->setText(name);
        }
        else
        {
            name = _obj->getDataObjectName();
            _ui->lineEdit_Name->setText(name);
            setDataToWidget();
        }
    }

    void CylinderInfoWidget::on_pushButton_OriginPoint_clicked()
    {
    }

    void CylinderInfoWidget::on_pushButton_AxisPoint_clicked()
    {
    }

    void CylinderInfoWidget::on_pushButton_Cancel_clicked()
    {
        if (_oper) {
            _oper->execProfession();
        }
    }

    void CylinderInfoWidget::on_pushButton_CreateOrEdit_clicked()
    {
        if (checkValue() == false)return;

        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        if (_isCreate) {
            QString name = _ui->lineEdit_Name->text();
            if (geometryData->getDataByName(name)) {
                QMessageBox::warning(this, "", tr("\"%1\" already exists and cannot be overwritten.").arg(name), QMessageBox::Ok);
                return;
            }

            Interface::FITKInterfaceGeometryFactory* geofactory = Interface::FITKInterfaceGeometryFactory::getInstance();
            if (geofactory == nullptr)return;

            _obj = dynamic_cast<Interface::FITKAbsGeoModelCylinder*>(geofactory->createCommand(Interface::FITKGeoEnum::FITKGeometryComType::FGTCylinder));
            if (_obj == nullptr)return;
            getDataFormWidget();
            _obj->setDataObjectName(name);
            _obj->update();
            geometryData->appendDataObj(_obj);
        }
        else {
            if (_obj == nullptr)return;
            getDataFormWidget();
            _obj->update();
        }

        if (_oper && _obj) {
            _oper->setArgs("objID", _obj->getDataObjectID());
            _oper->execProfession();
        }
    }

    bool CylinderInfoWidget::checkValue()
    {
        return true;
    }

    void CylinderInfoWidget::setDataToWidget()
    {
        if (_obj == nullptr)return;
        double originPoint[3] = { 0,0,0 };
        _obj->getLocation(originPoint);
        _ui->lineEdit_OriginPoint1->setText(QString::number(originPoint[0]));
        _ui->lineEdit_OriginPoint2->setText(QString::number(originPoint[1]));
        _ui->lineEdit_OriginPoint3->setText(QString::number(originPoint[2]));

        double axisPoint[3] = { 0,0,0 };
        _obj->getDirection(axisPoint);
        _ui->lineEdit_AxisPoint1->setText(QString::number(axisPoint[0]));
        _ui->lineEdit_AxisPoint2->setText(QString::number(axisPoint[1]));
        _ui->lineEdit_AxisPoint3->setText(QString::number(axisPoint[2]));

        double radius = _obj->getRadius();
        _ui->lineEdit_Radius->setText(QString::number(radius));

        double length = _obj->getLength();
        _ui->lineEdit_Length->setText(QString::number(length));
    }

    void CylinderInfoWidget::getDataFormWidget()
    {
        if (_obj == nullptr)return;

        double originPoint[3] = { 0,0,0 };
        originPoint[0] = _ui->lineEdit_OriginPoint1->text().toDouble();
        originPoint[1] = _ui->lineEdit_OriginPoint2->text().toDouble();
        originPoint[2] = _ui->lineEdit_OriginPoint3->text().toDouble();
        _obj->setLocation(originPoint);

        double axisPoint[3] = { 0,0,0 };
        axisPoint[0] = _ui->lineEdit_AxisPoint1->text().toDouble();
        axisPoint[1] = _ui->lineEdit_AxisPoint2->text().toDouble();
        axisPoint[2] = _ui->lineEdit_AxisPoint3->text().toDouble();
        _obj->setDirection(axisPoint);

        double radius = _ui->lineEdit_Radius->text().toDouble();
        _obj->setRadius(radius);

        double length = _ui->lineEdit_Length->text().toDouble();
        _obj->setLength(length);
    }
}
