#include "CylinderInfoWidget.h"
#include "ui_CylinderInfoWidget.h"
#include "CompFaceGroupWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIWidget/GUIPickInfo.h"
#include "GUIWidget/PickedDataProvider.h"
#include "GUIWidget/PickedData.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "OperatorsInterface/GraphEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelCylinder.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoShapeAgent.h"
#include "FITK_Interface/FITKInterfaceModel/FITKComponentManager.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"

#include <QMessageBox>
#include <QtMath>

#define CylObjID Qt::UserRole

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
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Edit"));
        _ui->lineEdit_Name->setEnabled(false);
    }

    CylinderInfoWidget::~CylinderInfoWidget()
    {
        clearTableWidget();

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

    void CylinderInfoWidget::setFaceGroupValue(int rowIndex, QList<int> facesId)
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        CompFaceGroupWidget* item = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(rowIndex, 0));
        if (item == nullptr)return;
        auto obj = commanger->getDataByID(item->data(CylObjID).toInt());
        if (obj == nullptr)return;

        //重新设置面id
        QString name = obj->getDataObjectName();
        obj->setMember(facesId);
        if (facesId.size() == 0) {
            name += tr("(empty)");
        }
        else {
            name += tr("(%1 faces)").arg(facesId.size());
        }
        item->setName(name);

        //处理其他模块
        for (int faceId : facesId) {
            for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
                if (i == rowIndex)continue;
                CompFaceGroupWidget* otherItem = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(i, 0));
                if (otherItem == nullptr)continue;
                auto otherObj = commanger->getDataByID(otherItem->data(CylObjID).toInt());
                if (otherObj == nullptr)continue;
                QList<int> ids = otherObj->getMember();

                //如果已经包含该id,移除该id，并重新设置名称
                if (ids.contains(faceId)) {
                    ids.removeOne(faceId);

                    QString otherName = otherObj->getDataObjectName();
                    otherObj->setMember(ids);
                    if (ids.size() == 0) {
                        otherName += tr("(empty)");
                    }
                    else {
                        otherName += tr("(%1 faces)").arg(ids.size());
                    }
                    otherItem->setName(otherName);
                }
            }
        }
    }

    Interface::FITKAbsGeoCommand * CylinderInfoWidget::getCurrentGeoCommand()
    {
        return _obj;
    }

    void CylinderInfoWidget::on_pushButton_OriginPoint_clicked()
    {
        if (_oper) {
            _oper->moveToStep(0);
        }
    }

    void CylinderInfoWidget::on_pushButton_Cancel_clicked()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        propertyWidget->init();
    }

    void CylinderInfoWidget::on_pushButton_CreateOrEdit_clicked()
    {
        if (checkValue() == false)return;

        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
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

            //切换为编辑模式
            _ui->groupBox_FaceGroups->show();
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

    void CylinderInfoWidget::on_pushButton_Clear_clicked()
    {
        _ui->tableWidget->clear();
        initTableWidget();
    }

    void CylinderInfoWidget::on_pushButton_Add_clicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        int rowNum = _ui->tableWidget->rowCount();
        _ui->tableWidget->setRowCount(rowNum + 1);

        QString group = tr("Group_%1").arg(rowNum + 1);
        QString name = group + tr("(empty)");
        QList<int> faceList = {};

        //创建面组对象
        Interface::FITKGeoComponent* geoCom = new Interface::FITKGeoComponent(Interface::FITKModelEnum::FITKModelSetType::FMSSurface);
        geoCom->setDataObjectName(group);
        commanger->appendDataObj(geoCom);

        CompFaceGroupWidget* widget = new CompFaceGroupWidget(_ui->tableWidget);
        widget->setName(name);
        widget->setData(CylObjID, geoCom->getDataObjectID());
        _ui->tableWidget->setCellWidget(rowNum, 0, widget);

        connect(widget, SIGNAL(sigEditNameStart()), this, SLOT(slotEditNameStart()));
        connect(widget, SIGNAL(sigEditNameFinish()), this, SLOT(slotEditNameFinish()));
        connect(widget, SIGNAL(sigOkClicked()), this, SLOT(slotFaceWidgetOkClicked()));
        connect(widget, SIGNAL(sigCancelClicked()), this, SLOT(slotFaceWidgetCancelClicked()));
        connect(widget, SIGNAL(sigDeleteClicked()), this, SLOT(slotFaceWidgetDeleteClicked()));

        //更新位置
        updateFaceWidgetCurrentPos();
    }

    void CylinderInfoWidget::slotCellTableClicked(int row, int column)
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(row, column));
        if (widget == nullptr)return;
        auto obj = commanger->getDataByID(widget->data(CylObjID).toInt());
        if (obj == nullptr)return;

        if (_oper) {
            setAllFaceGroupSelect(false);
            widget->setSelect(true);

            clearGraphHight();

            //执行操作器
            _oper->setArgs("objID", _obj->getDataObjectID());
            _oper->setArgs("faceIDs", QVariant::fromValue(obj->getMember()));
            _oper->moveToStep(1);
        }
    }

    void CylinderInfoWidget::slotEditNameStart()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(sender());
        if (widget == nullptr) return;

        auto obj = commanger->getDataByID(widget->data(CylObjID).toInt());
        if (obj == nullptr)return;
        widget->setName(obj->getDataObjectName());
    }

    void CylinderInfoWidget::slotEditNameFinish()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(sender());
        if (widget == nullptr) return;
        auto obj = commanger->getDataByID(widget->data(CylObjID).toInt());
        if (obj == nullptr)return;
        int objID = obj->getDataObjectID();
        QString name = "";
        //判断新名称是否存在
        if (commanger->getDataByName(widget->getName())) {
            name = obj->getDataObjectName();
        }
        else {
            name = widget->getName();
        }

        //设置控件名称
        obj->setDataObjectName(name);
        if (obj->getMember().size() == 0) {
            name += tr("(empty)");
        }
        else {
            name += tr("(%1 faces)").arg(obj->getMember().size());
        }
        widget->setName(name);

        //更新网格边界对象对应的名称
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return;
        auto meshSizeManger = Interface::FITKMeshGenInterface::getInstance()->getGeometryMeshSizeManager();
        for (int i = 0; i < meshSizeManger->getDataCount(); i++) {
            auto meshSizeObj = meshSizeManger->getDataByIndex(i);
            if (meshSizeObj == nullptr)continue;
            if (meshSizeObj->getGeoGroupComponentId() == objID) {
                QString meshSizeNewName = _obj->getDataObjectName() + "." + obj->getDataObjectName();
                meshSizeObj->setDataObjectName(meshSizeNewName);
                treeOper->updateTree();
                break;
            }
        }
    }

    void CylinderInfoWidget::slotFaceWidgetOkClicked()
    {
        CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(sender());
        if (widget == nullptr) return;
        //执行选择结束事件
        if (_oper) {
            _oper->setArgs("curRow", widget->getCurrentPos().first);
            _oper->moveToStep(2);
            widget->setSelect(false);
        }
    }

    void CylinderInfoWidget::slotFaceWidgetCancelClicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(sender());
        if (widget == nullptr) return;

        int objID = widget->data(CylObjID).toInt();
        commanger->removeDataByID(objID);
        _ui->tableWidget->removeRow(widget->getCurrentPos().first);

        //更新界面中存储的位置
        updateFaceWidgetCurrentPos();
        //清除高亮
        clearGraphHight();

        //判断当前面组是否被网格边界参数所使用，被使用移除对应的网格边界参数对象
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return;
        auto meshSizeManger = Interface::FITKMeshGenInterface::getInstance()->getGeometryMeshSizeManager();
        for (int i = 0; i < meshSizeManger->getDataCount(); i++) {
            auto meshSizeObj = meshSizeManger->getDataByIndex(i);
            if (meshSizeObj == nullptr)continue;
            if (meshSizeObj->getGeoGroupComponentId() == objID) {
                meshSizeManger->removeDataByID(meshSizeObj->getDataObjectID());
                treeOper->updateTree();
                break;
            }
        }
    }

    void CylinderInfoWidget::slotFaceWidgetDeleteClicked()
    {
        CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(sender());
        if (widget == nullptr) return;
        _ui->tableWidget->removeRow(widget->getCurrentPos().first);
        //更新界面中存储的位置
        updateFaceWidgetCurrentPos();
        //清除高亮
        clearGraphHight();
    }

    void CylinderInfoWidget::closeEvent(QCloseEvent * event)
    {
        clearGraphHight();
    }

    bool CylinderInfoWidget::checkValue()
    {
        auto outputMessage = [&](QString message) {
            QMessageBox::critical(nullptr, tr("Error Information"), message);
        };

        double axisPoint1 = _ui->lineEdit_Axis1->text().toDouble();
        double axisPoint2 = _ui->lineEdit_Axis2->text().toDouble();
        double axisPoint3 = _ui->lineEdit_Axis3->text().toDouble();
        if (axisPoint1 == 0 && axisPoint2 == 0 && axisPoint3 == 0) {
            outputMessage(tr("AxisX = 0 , AxisY = 0 , AxisZ = 0"));
            return false;
        }
        else
        {
            double mulRatio = qSqrt(axisPoint1*axisPoint1 + axisPoint2 * axisPoint2 + axisPoint3 * axisPoint3);
            _ui->lineEdit_Axis1->setText(QString::number(axisPoint1 / mulRatio));
            _ui->lineEdit_Axis2->setText(QString::number(axisPoint2 / mulRatio));
            _ui->lineEdit_Axis3->setText(QString::number(axisPoint3 / mulRatio));
        }

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

        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()-> getGeoComponentManager();
        if (commanger == nullptr)return;
        _ui->tableWidget->setRowCount(commanger->getDataCount());
        for (int i = 0; i < commanger->getDataCount(); i++) {
            Interface::FITKGeoComponent* geoCom = dynamic_cast<Interface::FITKGeoComponent*>(commanger->getDataByIndex(i));
            if (geoCom == nullptr)continue;
            QList<int> ids = geoCom->getMember();
            QString name = geoCom->getDataObjectName();

            CompFaceGroupWidget* item = new CompFaceGroupWidget(_ui->tableWidget);
            _ui->tableWidget->setCellWidget(i, 0, item);
            if (ids.size() == 0) {
                name += tr("(empty)");
            }
            else {
                name += tr("(%1 faces)").arg(ids.size());
            }
            item->setName(name);
            item->setData(CylObjID, geoCom->getDataObjectID());
            item->setCurrentPos(i, 0);

            connect(item, SIGNAL(sigEditNameStart()), this, SLOT(slotEditNameStart()));
            connect(item, SIGNAL(sigEditNameFinish()), this, SLOT(slotEditNameFinish()));
            connect(item, SIGNAL(sigOkClicked()), this, SLOT(slotFaceWidgetOkClicked()));
            connect(item, SIGNAL(sigCancelClicked()), this, SLOT(slotFaceWidgetCancelClicked()));
            connect(item, SIGNAL(sigDeleteClicked()), this, SLOT(slotFaceWidgetDeleteClicked()));
        }
        updateFaceWidgetCurrentPos();
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
    }

    void CylinderInfoWidget::initTableWidget()
    {
        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);
        QStringList header;
        header << tr("Default(3 faces)");
        _ui->tableWidget->setHorizontalHeaderLabels(header);
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //充满表格
        _ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        //设置只能单选
        _ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

        connect(_ui->tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotCellTableClicked(int, int)));
    }

    void CylinderInfoWidget::setAllFaceGroupSelect(bool type)
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            widget->setSelect(false);
        }
    }

    void CylinderInfoWidget::updateFaceWidgetCurrentPos()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            widget->setCurrentPos(i, 0);
        }
    }

    void CylinderInfoWidget::clearGraphHight()
    {
        //退出选择模式
        GraphData::PickedDataProvider* pickD = GraphData::PickedDataProvider::getInstance();
        if (pickD == nullptr) return;
        //拾取信息设置
        GUI::GUIPickInfoStru pinfo;
        pinfo._pickObjType = GUI::GUIPickInfo::PickObjType::POBJNone;
        pinfo._pickMethod = GUI::GUIPickInfo::PickMethod::PMNone;
        GUI::GUIPickInfo::SetPickInfo(pinfo);
        pickD->clearPickedData();

        //刷新渲染窗口
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->reRender();
    }

    void CylinderInfoWidget::clearTableWidget()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            QWidget* widget = _ui->tableWidget->cellWidget(i, 0);
            if (widget == nullptr)continue;
            delete widget;
            widget = nullptr;
        }

        _ui->tableWidget->clear();
    }
}
