#include "MeshBaseWidget.h"
#include "ui_MeshBaseWidget.h"
#include "MeshBaseTypeBoxWidget.h"
#include "MeshBaseTypeCylinderWidget.h"

#include "GUIFrame/MainWindow.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshSizeInfoGenerator.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeBox.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeCylinder.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeSphere.h"

namespace GUI
{
    MeshBaseWidget::MeshBaseWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _oper(oper)
    {
        _ui = new Ui::MeshBaseWidget();
        _ui->setupUi(this);
        _meshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getRegionMeshSizeMgr();
        
        init();
    }

    MeshBaseWidget::~MeshBaseWidget()
    {
        if (_ui)delete _ui;
        if (_subWidget)delete _subWidget;
    }

    void MeshBaseWidget::init()
    {
        //初始化type
        _ui->comboBox_Type->addItem(tr("Box"), Interface::FITKAbstractRegionMeshSize::RegionType::RegionBox);
        _ui->comboBox_Type->addItem(tr("Cylinder"), Interface::FITKAbstractRegionMeshSize::RegionType::RegionCylinder);

        //默认类型为box
        Interface::FITKAbstractRegionMeshSize::RegionType type = Interface::FITKAbstractRegionMeshSize::RegionBox;
        //获取第一位数据
        _currentObj = _meshSizeManager->getDataByIndex(0);
        if (_currentObj) {
            type = _currentObj->getRegionType();
        }

        switch (type) {
        case Interface::FITKAbstractRegionMeshSize::RegionBox: _subWidget = new MeshBaseTypeBoxWidget(); break;
        case Interface::FITKAbstractRegionMeshSize::RegionCylinder: _subWidget = new MeshBaseTypeCylinderWidget; break;
        case Interface::FITKAbstractRegionMeshSize::RegionSphere: break;
        }
        _ui->comboBox_Type->setCurrentIndex(_ui->comboBox_Type->findData(type));

        _subWidget->setDataToWidget(_currentObj);
        _ui->gridLayout_SubWidget->addWidget(_subWidget);
    }

    void MeshBaseWidget::on_comboBox_Type_activated(int index)
    {
        Interface::FITKAbstractRegionMeshSize::RegionType type = _ui->comboBox_Type->currentData().value<Interface::FITKAbstractRegionMeshSize::RegionType>();

        switch (type){
        case Interface::FITKAbstractRegionMeshSize::RegionBox: {
            QList<Interface::FITKAbstractRegionMeshSize*> meshSizeList = _meshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RegionBox);
            if (meshSizeList.size() != 0) {
                _currentObj = meshSizeList[0];
            }
            else
            {
                _currentObj = nullptr;
            }
            //重新添加界面
            updateWidget(new MeshBaseTypeBoxWidget());
            break;
        }
        case Interface::FITKAbstractRegionMeshSize::RegionCylinder: {
            QList<Interface::FITKAbstractRegionMeshSize*> meshSizeList = _meshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RegionCylinder);
            if (meshSizeList.size() != 0) {
                _currentObj = meshSizeList[0];
            }
            else
            {
                _currentObj = nullptr;
            }
            //重新添加界面
            updateWidget(new MeshBaseTypeCylinderWidget());
            break;
        }
        case Interface::FITKAbstractRegionMeshSize::RegionSphere: {
            QList<Interface::FITKAbstractRegionMeshSize*> meshSizeList = _meshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RegionSphere);
            if (meshSizeList.size() != 0) {
                _currentObj = meshSizeList[0];
            }
            else
            {
                _currentObj = nullptr;
            }
            break;
        }
        }
    }

    void MeshBaseWidget::on_pushButton_Cancel_clicked()
    {
        if (_oper) {
            _oper->execProfession();
        }
    }

    void MeshBaseWidget::on_pushButton_OK_clicked()
    {
        if (_subWidget->checkValue() == false)return;

        if (!_currentObj) {
            auto meshGenerator = Interface::FITKMeshGenInterface::getInstance()->getMeshSizeGenerator();

            Interface::FITKAbstractRegionMeshSize::RegionType type = 
                _ui->comboBox_Type->currentData().value<Interface::FITKAbstractRegionMeshSize::RegionType>();
            switch (type){
            case Interface::FITKAbstractRegionMeshSize::RegionBox:
                _currentObj = meshGenerator->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionBox); 
                break;
            case Interface::FITKAbstractRegionMeshSize::RegionCylinder:
                _currentObj = meshGenerator->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionCylinder);
                break;
            case Interface::FITKAbstractRegionMeshSize::RegionSphere:
                break;
            }
            _subWidget->getDataFromWidget(_currentObj);

            _meshSizeManager->insertDataObj(0, _currentObj);
        }
        else{
            _subWidget->getDataFromWidget(_currentObj);
            //移除对象但不释放内存
            _meshSizeManager->removeDataObjWithoutRelease(_currentObj);
            //插入到首位
            _meshSizeManager->insertDataObj(0, _currentObj);
        }

        if (_oper) {
            _oper->execProfession();
        }
    }

    void MeshBaseWidget::updateWidget(MeshBaseTypeWidgetBase* newWidget)
    {
        if (newWidget == nullptr)return;
        //重新添加界面
        if (_subWidget) {
            _ui->gridLayout_SubWidget->removeWidget(_subWidget);
            delete _subWidget;
            _subWidget = nullptr;
        }
        _subWidget = newWidget;
        _subWidget->setDataToWidget(_currentObj);
        _ui->gridLayout_SubWidget->addWidget(_subWidget);
    }
}


