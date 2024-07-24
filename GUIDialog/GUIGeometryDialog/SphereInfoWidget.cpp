#include "SphereInfoWidget.h"
#include "ui_SphereInfoWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelSphere.h"

#include <QMessageBox>

namespace GUI {

    SphereInfoWidget::SphereInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Create"));
    }

    SphereInfoWidget::SphereInfoWidget(Interface::FITKAbsGeoModelSphere * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(false), _obj(obj), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Edit"));
        _ui->lineEdit_Name->setEnabled(false);
    }

    SphereInfoWidget::~SphereInfoWidget()
    {
        if (_ui)delete _ui;
    }

    void SphereInfoWidget::init()
    {
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        _ui = new Ui::SphereInfoWidget();
        _ui->setupUi(this);

        QString name = "";
        if (_isCreate) {
            name = QString(tr("Sphere-%1").arg(geometryData->getDataCount() + 1));
            _ui->lineEdit_Name->setText(name);
        }
        else
        {
            name = _obj->getDataObjectName();
            _ui->lineEdit_Name->setText(name);
            setDataToWidget();
        }
    }

    void SphereInfoWidget::setCenterPoint(double * point)
    {
        _ui->lineEdit_CenterPoint1->setText(QString::number(point[0]));
        _ui->lineEdit_CenterPoint2->setText(QString::number(point[1]));
        _ui->lineEdit_CenterPoint3->setText(QString::number(point[2]));
    }

    void SphereInfoWidget::on_pushButton_CenterPoint_clicked()
    {
        if (_oper) {
            _oper->moveToStep(0);
        }
    }

    void SphereInfoWidget::on_pushButton_Cancel_clicked()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        propertyWidget->init();
    }

    void SphereInfoWidget::on_pushButton_CreateOrEdit_clicked()
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

            _obj = dynamic_cast<Interface::FITKAbsGeoModelSphere*>(geofactory->createCommand(Interface::FITKGeoEnum::FITKGeometryComType::FGTSphere));
            if (_obj == nullptr)return;
            getDataFormWidget();
            _obj->setDataObjectName(name);
            _obj->update();
            geometryData->appendDataObj(_obj);

            //模式切换
            _ui->pushButton_CreateOrEdit->setText(tr("Edit"));
            _ui->lineEdit_Name->setEnabled(false);
            _isCreate = false;
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

    bool SphereInfoWidget::checkValue()
    {
        return true;
    }

    void SphereInfoWidget::setDataToWidget()
    {
        if (_obj == nullptr)return;

        double centerPoint[3] = { 0,0,0 };
        _obj->getLocation(centerPoint);
        _ui->lineEdit_CenterPoint1->setText(QString::number(centerPoint[0]));
        _ui->lineEdit_CenterPoint2->setText(QString::number(centerPoint[1]));
        _ui->lineEdit_CenterPoint3->setText(QString::number(centerPoint[2]));

        double radius = _obj->getRadius();
        _ui->lineEdit_Radius->setText(QString::number(radius));
    }

    void SphereInfoWidget::getDataFormWidget()
    {
        if (_obj == nullptr)return;

        double centerPoint[3] = { 0,0,0 };
        centerPoint[0] = _ui->lineEdit_CenterPoint1->text().toDouble();
        centerPoint[1] = _ui->lineEdit_CenterPoint2->text().toDouble();
        centerPoint[2] = _ui->lineEdit_CenterPoint3->text().toDouble();
        _obj->setLocation(centerPoint);

        double radius = 0.0;
        radius = _ui->lineEdit_Radius->text().toDouble();
        _obj->setRadius(radius);
    }
}
