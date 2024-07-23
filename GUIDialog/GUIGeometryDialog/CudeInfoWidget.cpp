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
#include <QTableWidgetItem>

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
        _isCreate(false), _obj(obj), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Edit"));
        _ui->lineEdit_Name->setEnabled(false);
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

        initTableWidget();

        
        QString name = "";
        if (_isCreate) {
            name = QString(tr("Box-%1").arg(geometryData->getDataCount() + 1));
            _ui->lineEdit_Name->setText(name);
        }
        else
        {
            name = _obj->getDataObjectName();
            _ui->lineEdit_Name->setText(name);
            setDataToWidget();
        }

        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/icoR_selectBlue.png"), QSize(), QIcon::Normal, QIcon::Off);
        _ui->pushButton_BasicPoint->setIcon(icon1);
    }

    void CudeInfoWidget::setBasicPoint(double * point)
    {
        _ui->lineEdit_BasicPoint1->setText(QString::number(point[0]));
        _ui->lineEdit_BasicPoint2->setText(QString::number(point[1]));
        _ui->lineEdit_BasicPoint3->setText(QString::number(point[2]));
    }

    void CudeInfoWidget::updateTableWidget()
    {

    }

    void CudeInfoWidget::on_pushButton_BasicPoint_clicked()
    {
        if (_oper) {
            _oper->moveToStep(0);
        }
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

    void CudeInfoWidget::on_pushButton_Clear_clicked()
    {
        _ui->tableWidget->clear();
        initTableWidget();
    }

    void CudeInfoWidget::on_pushButton_Add_clicked()
    {
        int rowNum = _ui->tableWidget->rowCount();
        _ui->tableWidget->setRowCount(rowNum + 1);

        QString name = tr("Group_%1 (empty)").arg(rowNum + 1);
        QTableWidgetItem* item = new QTableWidgetItem(name);
        _ui->tableWidget->setItem(rowNum, 0, item);

        item = new QTableWidgetItem();
        item->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
        _ui->tableWidget->setItem(rowNum, 1, item);
    }

    void CudeInfoWidget::itemTableClickedSlot(QTableWidgetItem * item)
    {
        int curRow = _ui->tableWidget->currentRow();
        int curCol = _ui->tableWidget->currentColumn();

        if (curCol == 0) {
            if (_oper) {
                _oper->moveToStep(1);
            }
        }
        //删除操作
        else if (curCol == 1) {
            _ui->tableWidget->removeRow(curCol);
        }
    }

    void CudeInfoWidget::itemTableDoubleClickedSlot(QTableWidgetItem * item)
    {
        int curRow = _ui->tableWidget->currentRow();
        int curCol = _ui->tableWidget->currentColumn();
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

    void CudeInfoWidget::initTableWidget()
    {
        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(2);
        QStringList header;
        header << tr("Default(6 faces)") << tr("");
        _ui->tableWidget->setHorizontalHeaderLabels(header);
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

        on_pushButton_Add_clicked();

        connect(_ui->tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemTableClickedSlot(QTableWidgetItem*)));
        connect(_ui->tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(itemTableDoubleClickedSlot(QTableWidgetItem*)));
    }
}
