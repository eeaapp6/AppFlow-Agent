#include "CudeInfoWidget.h"
#include "ui_CudeInfoWidget.h"
#include "CompFaceGroupWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIWidget/GUIPickInfo.h"
#include "GUIWidget/PickedDataProvider.h"
#include "GUIWidget/PickedData.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"
#include "OperatorsInterface/GraphEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelBox.h"
#include "FITK_Interface/FITKInterfaceModel/FITKAbstractModel.h"
#include "FITK_Interface/FITKInterfaceModel/FITKComponentManager.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QSpacerItem>

#define CudeNamePos Qt::UserRole
#define CudeFacePos Qt::UserRole+1

namespace GUI {

    CudeInfoWidget::CudeInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true), _oper(oper)
    {
        init();

        _ui->pushButton_CreateOrEdit->setText(tr("Create"));
        _ui->groupBox_FaceGroups->hide();
    }

    CudeInfoWidget::CudeInfoWidget(Interface::FITKAbsGeoModelBox * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(false), _obj(obj), _oper(oper)
    {
        _geoModel = dynamic_cast<Interface::FITKAbstractGeoModel*>(_obj);

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

    void CudeInfoWidget::setFaceGroupValue(int rowIndex, QList<int> facesId)
    {
        CompFaceGroupWidget* item = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(rowIndex, 0));
        if (item == nullptr)return;

        //重新设置面id
        QString name = item->data(CudeNamePos).toString();
        item->setData(CudeFacePos, QVariant::fromValue(facesId));
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
                if (otherItem == nullptr)return;
                QList<int> ids = otherItem->data(CudeFacePos).value<QList<int>>();
                
                //如果已经包含该id,移除该id，并重新设置名称
                if (ids.contains(faceId)) {
                    ids.removeOne(faceId);

                    QString otherName = otherItem->data(CudeNamePos).toString();
                    otherItem->setData(CudeFacePos, QVariant::fromValue(ids));
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

    void CudeInfoWidget::on_pushButton_BasicPoint_clicked()
    {
        if (_oper) {
            _oper->moveToStep(0);
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

            //切换为编辑模式
            _ui->groupBox_FaceGroups->show();
            _ui->pushButton_CreateOrEdit->setText(tr("Edit"));
            _ui->lineEdit_Name->setEnabled(false);
            _isCreate = false;
            _geoModel = dynamic_cast<Interface::FITKAbstractGeoModel*>(_obj);
        }
        else {
            if (_obj == nullptr)return;
            getDataFormWidget();
            _obj->update();

            //更新几何划分网格尺寸数据
            updateMeshGeoMeshSize();
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

        QString group = tr("Group_%1").arg(rowNum + 1);
        QString name = group + tr("(empty)");
        QList<int> faceList = {};

        CompFaceGroupWidget* widget = new CompFaceGroupWidget(_ui->tableWidget);
        widget->setName(name);
        widget->setData(CudeNamePos, group);
        widget->setData(CudeFacePos, QVariant::fromValue(faceList));
        _ui->tableWidget->setCellWidget(rowNum, 0, widget);

        connect(widget, SIGNAL(sigOkClicked()), this, SLOT(slotFaceWidgetOkClicked()));
        connect(widget, SIGNAL(sigCancelClicked()), this, SLOT(slotFaceWidgetCancelClicked()));
        connect(widget, SIGNAL(sigDeleteClicked()), this, SLOT(slotFaceWidgetDeleteClicked()));

        updateFaceWidgetCurrentPos();
    }

    void CudeInfoWidget::slotCellTableClicked(int row, int column)
    {
        if (!_geoModel)return;

        CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(row, column));
        if (widget == nullptr)return;

        if (_oper) {
            setAllFaceGroupSelect(false);
            widget->setSelect(true);

            clearGraphHight();

            //执行操作器
            _oper->setArgs("objID", _geoModel->getDataObjectID());
            _oper->setArgs("faceIDs", widget->data(CudeFacePos));
            _oper->moveToStep(1);
        }
    }

    void CudeInfoWidget::slotFaceWidgetOkClicked()
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

    void CudeInfoWidget::slotFaceWidgetCancelClicked()
    {
        int currentRow = _ui->tableWidget->currentRow();
        CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(currentRow, 0));
        if (widget == nullptr) return;

        widget->setSelect(false);
        //清除高亮
        clearGraphHight();
    }

    void CudeInfoWidget::slotFaceWidgetDeleteClicked()
    {
        CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(sender());
        if (widget == nullptr) return;
        _ui->tableWidget->removeRow(widget->getCurrentPos().first);
        //更新界面中存储的位置
        updateFaceWidgetCurrentPos();
        //清除高亮
        clearGraphHight();
    }

    void CudeInfoWidget::closeEvent(QCloseEvent * event)
    {
        clearGraphHight();
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

        if (_geoModel == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _geoModel->getGeoComponentManager();
        if (commanger == nullptr)return;
        _ui->tableWidget->setRowCount(commanger->getDataCount());
        for (int i = 0; i < commanger->getDataCount(); i++) {
            Interface::FITKGeoComponent* geoCom = dynamic_cast<Interface::FITKGeoComponent*>(commanger->getDataByIndex(i));
            if (geoCom == nullptr)continue;
            QList<int> ids = geoCom->getMember();
            QString name = geoCom->getDataObjectName();

            CompFaceGroupWidget* item = new CompFaceGroupWidget(_ui->tableWidget);
            item->setData(CudeNamePos, name);
            item->setData(CudeFacePos, QVariant::fromValue(ids));
            _ui->tableWidget->setCellWidget(i, 0, item);
            if (ids.size() == 0) {
                name += tr("(empty)");
            }
            else {
                name += tr("(%1 faces)").arg(ids.size());
            }
            item->setName(name);
            item->setCurrentPos(i, 0);

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


        if (_geoModel == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _geoModel->getGeoComponentManager();
        if (commanger == nullptr)return;
        commanger->clear();
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompFaceGroupWidget* item = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (item == nullptr)return;
            QList<int> ids = item->data(CudeFacePos).value<QList<int>>();
            QString name = item->data(CudeNamePos).toString();
            Interface::FITKGeoComponent* geoCom = new Interface::FITKGeoComponent(Interface::FITKModelEnum::FITKModelSetType::FMSSurface);
            geoCom->setMember(ids);
            geoCom->setDataObjectName(name);
            commanger->appendDataObj(geoCom);
        }
    }

    void CudeInfoWidget::initTableWidget()
    {
        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);
        QStringList header;
        header << tr("Default(6 faces)");
        _ui->tableWidget->setHorizontalHeaderLabels(header);
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //充满表格
        _ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

        connect(_ui->tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotCellTableClicked(int, int)));
    }

    void CudeInfoWidget::setAllFaceGroupSelect(bool type)
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            widget->setSelect(false);
        }
    }

    void CudeInfoWidget::updateFaceWidgetCurrentPos()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompFaceGroupWidget* widget = dynamic_cast<CompFaceGroupWidget*>(_ui->tableWidget->cellWidget(i, 0));
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
    }

    void CudeInfoWidget::updateMeshGeoMeshSize()
    {
        //更新几何划分网格尺寸参数类面组id
        updateMeshGeoMeshSizeID();

        //更新面组id后，查看是否可通过id查找到模型
        QList<Interface::FITKGeometryMeshSize*> deleteList = {};
        Interface::FITKGeometryMeshSizeManager* manger = Interface::FITKMeshGenInterface::getInstance()->getGeometryMeshSizeManager();
        if (manger == nullptr)return;
        for (int i = 0; i < manger->getDataCount(); i++) {
            auto geoMeshSize = manger->getDataByIndex(i);
            if (geoMeshSize == nullptr)continue;
            //如果查询不到，认为面组被删除，清除几何划分网格尺寸参数对象
            if (!geoMeshSize->getGeoModel()) {
                deleteList.append(geoMeshSize);
            }
        }

        for (auto d : deleteList) {
            manger->removeDataObj(d);
        }
    }

    void CudeInfoWidget::updateMeshGeoMeshSizeID()
    {
        Interface::FITKGeometryMeshSizeManager* manger = Interface::FITKMeshGenInterface::getInstance()->getGeometryMeshSizeManager();
        if (manger == nullptr)return;

        Interface::FITKGeoComponentManager* commanger = _geoModel->getGeoComponentManager();
        if (commanger == nullptr)return;

        //获取该模型相关的面组列表
        QList<Interface::FITKGeometryMeshSize*> geoMeshSizeList = {};
        for (int j = 0; j < manger->getDataCount(); j++) {
            auto geoComp = manger->getDataByIndex(j);
            if (geoComp == nullptr)continue;
            if (geoComp->getDataObjectName().contains(_ui->lineEdit_Name->text())) geoMeshSizeList.append(geoComp);
        }

        //更新几何网格参数中的面组id
        for (int i = 0; i < commanger->getDataCount(); i++) {
            auto comp = commanger->getDataByIndex(i);
            if (comp == nullptr)continue;
            for (auto geoMesh : geoMeshSizeList) {
                QString name1 = _ui->lineEdit_Name->text() + "." + comp->getDataObjectName();
                QString name2 = geoMesh->getDataObjectName();
                if (name1 != name2)continue;
                geoMesh->setGeoGroupComponentId(comp->getDataObjectID());
            }
        }
    }
}
