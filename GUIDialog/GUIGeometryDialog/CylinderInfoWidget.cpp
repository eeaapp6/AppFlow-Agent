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

#define CylNamePos Qt::UserRole
#define CylFacePos Qt::UserRole+1

namespace GUI {

    CylinderInfoWidget::CylinderInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Create"));
        _ui->groupBox_FaceGroups->hide();
    }

    CylinderInfoWidget::CylinderInfoWidget(Interface::FITKAbsGeoModelCylinder * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(false), _obj(obj), _oper(oper)
    {
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;
        _geoModel = geometryData->getDataByID(_obj->getDataObjectID());

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

        initTableWidget();

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

        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/icoR_selectBlue.png"), QSize(), QIcon::Normal, QIcon::Off);
        _ui->pushButton_OriginPoint->setIcon(icon1);
    }

    void CylinderInfoWidget::setOriginPoint(double * point)
    {
        _ui->lineEdit_OriginPoint1->setText(QString::number(point[0]));
        _ui->lineEdit_OriginPoint2->setText(QString::number(point[1]));
        _ui->lineEdit_OriginPoint3->setText(QString::number(point[2]));
    }

    void CylinderInfoWidget::setFaceGroupValue(int rowIndex, int faceId)
    {
        //当前id处理
        {
            QTableWidgetItem* item = _ui->tableWidget->item(rowIndex, 0);
            if (item == nullptr)return;
            QList<int> ids = item->data(CylFacePos).value<QList<int>>();

            //如果已经包含该id不在添加
            if (!ids.contains(faceId)) {
                ids.append(faceId);
            }

            QString name = item->data(CylNamePos).toString();
            item->setData(CylFacePos, QVariant::fromValue(ids));
            name += tr("(%1 faces)").arg(ids.size());
            item->setText(name);
        }

        //处理其他模块
        {
            for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
                if (i == rowIndex)continue;
                QTableWidgetItem* item = _ui->tableWidget->item(i, 0);
                if (item == nullptr)return;
                QList<int> ids = item->data(CylFacePos).value<QList<int>>();
                //如果已经包含该id不在添加
                if (ids.contains(faceId)) {
                    ids.removeOne(faceId);

                    QString name = item->data(CylNamePos).toString();
                    item->setData(CylFacePos, QVariant::fromValue(ids));
                    if (ids.size() == 0) {
                        name += tr("(empty)");
                    }
                    else {
                        name += tr("(%1 faces)").arg(ids.size());
                    }
                    item->setText(name);
                }
            }
        }
    }

    void CylinderInfoWidget::on_pushButton_OriginPoint_clicked()
    {
        if (_oper) {
            _oper->moveToStep(0);
        }
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

    void CylinderInfoWidget::on_pushButton_Clear_clicked()
    {
        _ui->tableWidget->clear();
        initTableWidget();
    }

    void CylinderInfoWidget::on_pushButton_Add_clicked()
    {
        int rowNum = _ui->tableWidget->rowCount();
        _ui->tableWidget->setRowCount(rowNum + 1);

        QString group = tr("Group_%1").arg(rowNum + 1);
        QString name = group + tr("(empty)");
        QList<int> faceList = {};
        QTableWidgetItem* item = new QTableWidgetItem(name);
        item->setData(CylNamePos, group);
        item->setData(CylFacePos, QVariant::fromValue(faceList));
        _ui->tableWidget->setItem(rowNum, 0, item);

        item = new QTableWidgetItem();
        item->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
        _ui->tableWidget->setItem(rowNum, 1, item);
    }

    void CylinderInfoWidget::itemTableClickedSlot(QTableWidgetItem * item)
    {
        int curRow = _ui->tableWidget->currentRow();
        int curCol = _ui->tableWidget->currentColumn();

        if (curCol == 0) {
            if (!_geoModel)return;
            if (_oper) {
                _oper->setArgs("objID", _geoModel->getDataObjectID());
                _oper->setArgs("curRow", curRow);
                _oper->moveToStep(1);
            }
        }
        //删除操作
        else if (curCol == 1) {
            _ui->tableWidget->removeRow(curCol);
        }
    }

    void CylinderInfoWidget::itemTableDoubleClickedSlot(QTableWidgetItem * item)
    {
        int curRow = _ui->tableWidget->currentRow();
        int curCol = _ui->tableWidget->currentColumn();
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

        double axis[3] = { 0,0,0 };
        _obj->getDirection(axis);
        _ui->lineEdit_Axis1->setText(QString::number(axis[0]));
        _ui->lineEdit_Axis2->setText(QString::number(axis[1]));
        _ui->lineEdit_Axis3->setText(QString::number(axis[2]));

        double radius = _obj->getRadius();
        _ui->lineEdit_Radius->setText(QString::number(radius));

        double length = _obj->getLength();
        _ui->lineEdit_Length->setText(QString::number(length));

        if (_geoModel == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _geoModel->getGeoComponentManager();
        if (commanger == nullptr)return;
        _ui->tableWidget->setRowCount(commanger->getDataCount());
        for (int i = 0; i < commanger->getDataCount(); i++) {
            Interface::FITKGeoComponent* geoCom = dynamic_cast<Interface::FITKGeoComponent*>(commanger->getDataByIndex(i));
            if (geoCom == nullptr)continue;
            QList<int> ids = geoCom->getMember();
            QString name = geoCom->getDataObjectName();

            QTableWidgetItem* item = new QTableWidgetItem();
            item->setData(CylNamePos, name);
            item->setData(CylFacePos, QVariant::fromValue(ids));
            _ui->tableWidget->setItem(i, 0, item);
            if (ids.size() == 0) {
                name += tr("(empty)");
            }
            else {
                name += tr("(%1 faces)").arg(ids.size());
            }
            item->setText(name);

            item = new QTableWidgetItem();
            item->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
            _ui->tableWidget->setItem(i, 1, item);
        }
    }

    void CylinderInfoWidget::getDataFormWidget()
    {
        if (_obj == nullptr)return;

        double originPoint[3] = { 0,0,0 };
        originPoint[0] = _ui->lineEdit_OriginPoint1->text().toDouble();
        originPoint[1] = _ui->lineEdit_OriginPoint2->text().toDouble();
        originPoint[2] = _ui->lineEdit_OriginPoint3->text().toDouble();
        _obj->setLocation(originPoint);

        double axis[3] = { 0,0,0 };
        axis[0] = _ui->lineEdit_Axis1->text().toDouble();
        axis[1] = _ui->lineEdit_Axis2->text().toDouble();
        axis[2] = _ui->lineEdit_Axis3->text().toDouble();
        _obj->setDirection(axis);

        double radius = _ui->lineEdit_Radius->text().toDouble();
        _obj->setRadius(radius);

        double length = _ui->lineEdit_Length->text().toDouble();
        _obj->setLength(length);

        if (_geoModel == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _geoModel->getGeoComponentManager();
        if (commanger == nullptr)return;
        commanger->clear();
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            QTableWidgetItem* item = _ui->tableWidget->item(i, 0);
            if (item == nullptr)return;
            QList<int> ids = item->data(CylFacePos).value<QList<int>>();
            QString name = item->data(CylNamePos).toString();
            Interface::FITKGeoComponent* geoCom = new Interface::FITKGeoComponent(Interface::FITKModelEnum::FITKModelSetType::FMSSurface);
            geoCom->setMember(ids);
            geoCom->setDataObjectName(name);
            commanger->appendDataObj(geoCom);
        }
    }

    void CylinderInfoWidget::initTableWidget()
    {
        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(2);
        QStringList header;
        header << tr("Default(3 faces)") << tr("");
        _ui->tableWidget->setHorizontalHeaderLabels(header);
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

        connect(_ui->tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemTableClickedSlot(QTableWidgetItem*)));
        connect(_ui->tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(itemTableDoubleClickedSlot(QTableWidgetItem*)));
    }
}
