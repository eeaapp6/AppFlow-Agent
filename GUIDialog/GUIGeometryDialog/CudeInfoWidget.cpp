#include "CudeInfoWidget.h"
#include "ui_CudeInfoWidget.h"

#include "GUIFrame/MainWindow.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelBox.h"

#include <QMessageBox>

namespace GUI {
    
    CudeInfoWidget::CudeInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Create"));
    }

    CudeInfoWidget::CudeInfoWidget(Interface::FITKAbsGeoModelBox * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true), _obj(obj), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Edit"));
        _ui->lineEdit_Name->setEnabled(false);

        setDataToWidget();
    }

    CudeInfoWidget::~CudeInfoWidget()
    {
        if (_ui)delete _ui;
    }
    
    void CudeInfoWidget::init()
    {
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        _ui = new Ui::CudeInfoWidget();
        _ui->setupUi(this);

        QString name = "";
        if (_isCreate) {
            name = QString(tr("Box-%1").arg(geometryData->getDataCount() + 1));
            _ui->lineEdit_Name->setText(name);
        }
        else
        {
            //name = _obj->objectName();
            _ui->lineEdit_Name->setText(name);
        }
    }

    void CudeInfoWidget::on_pushButton_BasicPoint_clicked()
    {

    }

    void CudeInfoWidget::on_pushButton_Cancel_clicked()
    {
        if (_oper) {
            _oper->execProfession();
        }
    }

    void CudeInfoWidget::on_pushButton_CreateOrEdit_clicked()
    {
        if(checkValue() == false)return;

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

            _obj = dynamic_cast<Interface::FITKAbsGeoModelBox*>(geofactory->createCommand(Interface::FITKGeoEnum::FITKGeometryComType::FGTBox));
            if (_obj == nullptr)return;
            getDataFormWidget();
            _obj->update();
            geometryData->appendDataObj(_obj);
        }

        if (_oper && _obj) {
            _oper->setArgs("objID", _obj->getDataObjectID());
            _oper->execProfession();
        }
    }

    void CudeInfoWidget::on_pushButton_Clear_clicked()
    {

    }

    void CudeInfoWidget::on_pushButton_Add_clicked()
    {

    }

    bool CudeInfoWidget::checkValue()
    {
        return true;
    }

    void CudeInfoWidget::setDataToWidget()
    {
        if (_obj == nullptr)return;
        double basicPoint[3] = { 0,0,0 };
        _obj->getPoint1(basicPoint);
        _ui->lineEdit_BasicPoint1->setText(QString::number(basicPoint[0]));
        _ui->lineEdit_BasicPoint2->setText(QString::number(basicPoint[1]));
        _ui->lineEdit_BasicPoint3->setText(QString::number(basicPoint[2]));

        double dimensions[3] = { 0,0,0 };
        _obj->getLength(dimensions);
        _ui->lineEdit_Dimensions1->setText(QString::number(dimensions[0]));
        _ui->lineEdit_Dimensions2->setText(QString::number(dimensions[1]));
        _ui->lineEdit_Dimensions3->setText(QString::number(dimensions[2]));
    }

    void CudeInfoWidget::getDataFormWidget()
    {
        if (_obj == nullptr)return;

        double basicPoint[3] = { 0,0,0 };
        basicPoint[0] = _ui->lineEdit_BasicPoint1->text().toDouble();
        basicPoint[1] = _ui->lineEdit_BasicPoint2->text().toDouble();
        basicPoint[2] = _ui->lineEdit_BasicPoint3->text().toDouble();
        _obj->setPoint1(basicPoint);

        double dimensions[3] = { 0,0,0 };
        dimensions[0] = _ui->lineEdit_Dimensions1->text().toDouble();
        dimensions[1] = _ui->lineEdit_Dimensions2->text().toDouble();
        dimensions[2] = _ui->lineEdit_Dimensions3->text().toDouble();
        _obj->setLength(dimensions);
    }
}
