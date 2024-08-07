#include "CudeInfoWidget.h"
#include "ui_CudeInfoWidget.h"
#include "CompFaceGroupSelectWidget.h"

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
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelBox.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoShapeAgent.h"
#include "FITK_Interface/FITKInterfaceModel/FITKAbstractModel.h"
#include "FITK_Interface/FITKInterfaceModel/FITKComponentManager.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QSpacerItem>

#define CudeObjID Qt::UserRole

namespace GUI {

    CudeInfoWidget::CudeInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        GeometryWidgetBase(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Create"));
        _ui->groupBox_FaceGroups->hide();
    }

    CudeInfoWidget::CudeInfoWidget(Interface::FITKAbsGeoModelBox * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        GeometryWidgetBase(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(false), _obj(obj), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Edit"));
        _ui->lineEdit_Name->setEnabled(false);
    }

    CudeInfoWidget::~CudeInfoWidget()
    {
        clearTableWidget();
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
            name = geometryData->checkName(tr("Box-1"));
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

    void CudeInfoWidget::setFaceGroupValue(int rowIndex, QList<int> facesId)
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        CompFaceGroupSelectWidget* item = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(rowIndex, 0));
        if (item == nullptr)return;
        auto obj = commanger->getDataByID(item->data(CudeObjID).toInt());
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
                CompFaceGroupSelectWidget* otherItem = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
                if (otherItem == nullptr)continue;
                auto otherObj = commanger->getDataByID(otherItem->data(CudeObjID).toInt());
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

    Interface::FITKAbsGeoCommand * CudeInfoWidget::getCurrentGeoCommand()
    {
        return _obj;
    }

    void CudeInfoWidget::closeEvent(QCloseEvent * event)
    {
        GeometryWidgetBase::closeEvent(event);
        clearGraphHight();
    }

    void CudeInfoWidget::on_pushButton_BasicPoint_clicked()
    {
        if (_oper) {
            _oper->eventProcess(0);
        }
    }

    void CudeInfoWidget::on_pushButton_Cancel_clicked()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        propertyWidget->init();
    }

    void CudeInfoWidget::on_pushButton_CreateOrEdit_clicked()
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

            _obj = dynamic_cast<Interface::FITKAbsGeoModelBox*>(geofactory->createCommand(Interface::FITKGeoEnum::FITKGeometryComType::FGTBox));
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

    void CudeInfoWidget::on_pushButton_Clear_clicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        for (int i = 0; i < _ui->tableWidget->rowCount(); i++){
            CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            commanger->removeDataByID(widget->data(CudeObjID).toInt());
        }

        clearTableWidget();
        clearGraphHight();
    }

    void CudeInfoWidget::on_pushButton_Add_clicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        int rowNum = _ui->tableWidget->rowCount();
        _ui->tableWidget->setRowCount(rowNum + 1);

        QString group = commanger->checkName(tr("Group-1"));
        QString name = group + tr("(empty)");

        //创建面组对象
        Interface::FITKGeoComponent* geoCom = new Interface::FITKGeoComponent(Interface::FITKModelEnum::FITKModelSetType::FMSSurface);
        geoCom->setDataObjectName(group);
        commanger->appendDataObj(geoCom);

        CompFaceGroupSelectWidget* widget = new CompFaceGroupSelectWidget(_ui->tableWidget);
        widget->setName(name);
        widget->setData(CudeObjID, geoCom->getDataObjectID());
        _ui->tableWidget->setCellWidget(rowNum, 0, widget);

        connect(widget, SIGNAL(sigEditNameStart()), this, SLOT(slotEditNameStart()));
        connect(widget, SIGNAL(sigEditNameFinish()), this, SLOT(slotEditNameFinish()));
        connect(widget, SIGNAL(sigOkClicked()), this, SLOT(slotFaceWidgetOkClicked()));
        connect(widget, SIGNAL(sigCancelClicked()), this, SLOT(slotFaceWidgetCancelClicked()));
        connect(widget, SIGNAL(sigDeleteClicked()), this, SLOT(slotFaceWidgetDeleteClicked()));

        //更新位置
        updateFaceWidgetCurrentPos();
    }

    void CudeInfoWidget::slotCellTableClicked(int row, int column)
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(row, column));
        if (widget == nullptr)return;
        auto obj = commanger->getDataByID(widget->data(CudeObjID).toInt());
        if (obj == nullptr)return;

        if (_oper) {
            setAllFaceGroupSelect(false);
            widget->setSelect(true);
            clearGraphHight();
            //执行操作器
            _oper->setArgs("objID", _obj->getDataObjectID());
            _oper->setArgs("faceIDs", QVariant::fromValue(obj->getMember()));
            _oper->eventProcess(1);
        }
    }

    void CudeInfoWidget::slotEditNameStart()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(sender());
        if (widget == nullptr) return;

        auto obj = commanger->getDataByID(widget->data(CudeObjID).toInt());
        if (obj == nullptr)return;
        widget->setName(obj->getDataObjectName());
    }

    void CudeInfoWidget::slotEditNameFinish()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(sender());
        if (widget == nullptr) return;
        auto obj = commanger->getDataByID(widget->data(CudeObjID).toInt());
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

    void CudeInfoWidget::slotFaceWidgetOkClicked()
    {
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(sender());
        if (widget == nullptr) return;
        //执行选择结束事件
        if (_oper) {
            _oper->setArgs("curRow", widget->getCurrentPos().first);
            _oper->eventProcess(2);
            widget->setSelect(false);
        }

        _ui->tableWidget->setCurrentCell(-1, -1);
    }

    void CudeInfoWidget::slotFaceWidgetCancelClicked()
    {
        int currentRow = _ui->tableWidget->currentRow();
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(currentRow, 0));
        if (widget == nullptr) return;

        widget->setSelect(false);
        //清除高亮
        clearGraphHight();

        _ui->tableWidget->setCurrentCell(-1, -1);
    }

    void CudeInfoWidget::slotFaceWidgetDeleteClicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(sender());
        if (widget == nullptr) return;

        int objID = widget->data(CudeObjID).toInt();
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

        _ui->tableWidget->setCurrentCell(-1, -1);
    }

    bool CudeInfoWidget::checkValue()
    {
        auto outputMessage = [&](QString name,double value) {
            QMessageBox::critical(this, "", tr("%1 value : %2 error!").arg(name).arg(value), QMessageBox::Ok);
        };

        double value = _ui->lineEdit_Dimensions1->text().toDouble();
        if (value == 0) {
            outputMessage(tr("Dimensions X"), value);
            return false;
        }

        value = _ui->lineEdit_Dimensions2->text().toDouble();
        if (value == 0) {
            outputMessage(tr("Dimensions Y"), value);
            return false;
        }

        value = _ui->lineEdit_Dimensions3->text().toDouble();
        if (value == 0) {
            outputMessage(tr("Dimensions Z"), value);
            return false;
        }

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

        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        _ui->tableWidget->setRowCount(commanger->getDataCount());
        for (int i = 0; i < commanger->getDataCount(); i++) {
            Interface::FITKGeoComponent* geoCom = dynamic_cast<Interface::FITKGeoComponent*>(commanger->getDataByIndex(i));
            if (geoCom == nullptr)continue;
            QList<int> ids = geoCom->getMember();
            QString name = geoCom->getDataObjectName();

            CompFaceGroupSelectWidget* item = new CompFaceGroupSelectWidget(_ui->tableWidget);
            _ui->tableWidget->setCellWidget(i, 0, item);
            if (ids.size() == 0) {
                name += tr("(empty)");
            }
            else {
                name += tr("(%1 faces)").arg(ids.size());
            }
            item->setName(name);
            item->setData(CudeObjID, geoCom->getDataObjectID());
            item->setCurrentPos(i, 0);

            connect(item, SIGNAL(sigEditNameStart()), this, SLOT(slotEditNameStart()));
            connect(item, SIGNAL(sigEditNameFinish()), this, SLOT(slotEditNameFinish()));
            connect(item, SIGNAL(sigOkClicked()), this, SLOT(slotFaceWidgetOkClicked()));
            connect(item, SIGNAL(sigCancelClicked()), this, SLOT(slotFaceWidgetCancelClicked()));
            connect(item, SIGNAL(sigDeleteClicked()), this, SLOT(slotFaceWidgetDeleteClicked()));
        }
        updateFaceWidgetCurrentPos();
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

    //void CudeInfoWidget::updateTableTitle()
    //{
    //    if (_obj == nullptr)return;
    //    Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
    //    if (commanger == nullptr)return;

    //    //计算剩余面
    //    QList<int> allPoint = {};
    //    for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
    //        CompFaceGroupSelectWidget* otherItem = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
    //        if (otherItem == nullptr)continue;
    //        auto otherObj = commanger->getDataByID(otherItem->data(CudeObjID).toInt());
    //        if (otherObj == nullptr)continue;
    //        QList<int> ids = otherObj->getMember();
    //        allPoint.append(ids);
    //    }
    //    QStringList header;
    //    header << tr("Default(%1 faces)").arg(6 - allPoint.size());
    //    _ui->tableWidget->setHorizontalHeaderLabels(header);
    //}

    void CudeInfoWidget::initTableWidget()
    {
        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);

        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //充满表格
        _ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        //设置只能单选
        _ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        //隐藏行表头
        _ui->tableWidget->verticalHeader()->setVisible(false);
        //隐藏列表头
        _ui->tableWidget->horizontalHeader()->setVisible(false);

        connect(_ui->tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotCellTableClicked(int, int)));
    }

    void CudeInfoWidget::setAllFaceGroupSelect(bool type)
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            widget->setSelect(false);
        }
    }

    void CudeInfoWidget::updateFaceWidgetCurrentPos()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            widget->setCurrentPos(i, 0);
        }
    }

    void CudeInfoWidget::clearGraphHight()
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

    void CudeInfoWidget::clearTableWidget()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            QWidget* widget = _ui->tableWidget->cellWidget(i, 0);
            if (widget == nullptr)continue;
            delete widget;
            widget = nullptr;
        }

        _ui->tableWidget->clear();
        _ui->tableWidget->setRowCount(0);
    }
}
