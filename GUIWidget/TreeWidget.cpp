#include "TreeWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIFrame/MainTreeWidget.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"

#include <QMenu>
#include <QStandardItemModel>
#include <QTreeView>
#include <QTreeWidgetItem>
#include <QHeaderView>

Q_DECLARE_METATYPE(GUI::MainTreeEnum)

namespace GUI{

    // TreeWidget类构造函数
    TreeWidget::TreeWidget(QWidget* parent) :
        QTreeWidget(parent)
    {
        //右键 不可少否则右键无反应
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onItemClicked(QTreeWidgetItem*, int)));
        connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onModelCustomContextMenu(QPoint)));

        //隐藏列标题
        setHeaderHidden(true);
    }
    // TreeWidget类析构函数
    TreeWidget::~TreeWidget()
    {

    }

    // 更新所有模型案例树方法
    void TreeWidget::updateTree()
    {
        this->clear();

        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        QTreeWidgetItem* geometryItem = new QTreeWidgetItem();
        geometryItem->setText(0, "geometry");
        geometryItem->setExpanded(true);
        this->addTopLevelItem(geometryItem);

        for (int i = 0; i < geometryData->getDataCount(); i++) {
            auto geometryObj = dynamic_cast<Interface::FITKAbsGeoCommand*>(geometryData->getDataByIndex(i));
            if (geometryObj == nullptr)continue;

            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setExpanded(true);
            item->setText(0, geometryObj->getDataObjectName());
            item->setData(1, 0, geometryObj->getDataObjectID());

            GUI::MainTreeEnum treeType = GUI::MainTreeEnum::MainTree_None;
            Interface::FITKGeoEnum::FITKGeometryComType geometryType = geometryObj->getGeometryCommandType();
            switch (geometryType) {
            case Interface::FITKGeoEnum::FGTNone:break;
            case Interface::FITKGeoEnum::FGTBox:  treeType = GUI::MainTreeEnum::MainTree_GeometyBoxItem; break;
            case Interface::FITKGeoEnum::FGTCylinder:treeType = GUI::MainTreeEnum::MainTree_GeometyCylinderItem; break;
            case Interface::FITKGeoEnum::FGTSphere:treeType = GUI::MainTreeEnum::MainTree_GeometySphereItem;  break;
            }
            item->setData(2, 0, QVariant::fromValue(treeType));

            geometryItem->addChild(item);
        }

        //展开全部子集
        setItemsExpandable(true);		
        expandAll();
    }

    void TreeWidget::onItemClicked(QTreeWidgetItem * item, int column)
    {
        if (item == nullptr)return;
        int objID = item->data(1, 0).toInt();
        GUI::MainTreeEnum treeType = item->data(2, 0).value<GUI::MainTreeEnum>();
        Interface::FITKGeoEnum::FITKGeometryComType geometryType = item->data(3, 0).value<Interface::FITKGeoEnum::FITKGeometryComType>();

        QString name = "";

        switch (treeType) {
        case GUI::MainTreeEnum::MainTree_Geomety: break;
        case GUI::MainTreeEnum::MainTree_GeometyBoxItem:name = "actionEditCube"; break;
        case GUI::MainTreeEnum::MainTree_GeometyCylinderItem:name = "actionEditCylinder"; break;
        case GUI::MainTreeEnum::MainTree_GeometySphereItem:name = "actionEditSphere"; break;
        case GUI::MainTreeEnum::MainTree_Mesh: break;
        case GUI::MainTreeEnum::MainTree_MeshItem: break;
        }

        if (!name.isEmpty()) {
            QObject sender;
            sender.setObjectName(name);
            auto acOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<Core::FITKActionOperator>(name);
            if (acOper == nullptr)return;
            acOper->setEmitter(&sender);
            acOper->setArgs("objID", objID);
            acOper->actionTriggered();
        }
        else {
            auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
            if (treeOper == nullptr) return;
            treeOper->moveProcessToStep(0);
        }
    }

    void TreeWidget::onDoubleClicked(QTreeWidgetItem * item, int column)
    {
        if (item == nullptr)return;
        int objID = item->data(1, 0).toInt();
        GUI::MainTreeEnum treeType = item->data(2, 0).value<GUI::MainTreeEnum>();
        Interface::FITKGeoEnum::FITKGeometryComType geometryType = item->data(3, 0).value<Interface::FITKGeoEnum::FITKGeometryComType>();
    }

    void TreeWidget::onModelCustomContextMenu(QPoint point)
    {
        QTreeWidgetItem* item = this->currentItem();
        if (item == nullptr)return;

        int objID = item->data(1, 0).toInt();
        GUI::MainTreeEnum treeType = item->data(2, 0).value<GUI::MainTreeEnum>();
        Interface::FITKGeoEnum::FITKGeometryComType geometryType = item->data(3, 0).value<Interface::FITKGeoEnum::FITKGeometryComType>();

        QMenu menu;
        switch (treeType) {
        case GUI::MainTreeEnum::MainTree_Geomety: break;
        case GUI::MainTreeEnum::MainTree_GeometyBoxItem: {
            //addMenuActions(menu, "actionRenameCube", "Cube rename"); 
            addMenuActions(menu, "actionDeleteCube", "Cude delete"); 
            break; 
        }
        case GUI::MainTreeEnum::MainTree_GeometyCylinderItem: {
            //addMenuActions(menu, "actionRenameCylinder", "Cylinder rename");
            addMenuActions(menu, "actionDeleteCylinder", "Cylinder delete");
            break;
        }
        case GUI::MainTreeEnum::MainTree_GeometySphereItem: {
            //addMenuActions(menu, "actionRenameSphere", "Sphere rename");
            addMenuActions(menu, "actionDeleteSphere", "Sphere delete");
            break;
        }
        case GUI::MainTreeEnum::MainTree_Mesh: break;
        case GUI::MainTreeEnum::MainTree_MeshItem: break;
        }

        if (menu.actions().size() == 0) return;
        // 在鼠标点击位置执行上下文菜单
        menu.exec(QCursor::pos());
    }

    void TreeWidget::acitonClicked()
    {
        QTreeWidgetItem* item = this->currentItem();
        if (item == nullptr)return;
        int objID = item->data(1, 0).toInt();

        QObject* senderObject = sender();
        if (senderObject == nullptr)return;

        auto acOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<Core::FITKActionOperator>(senderObject->objectName());
        if (acOper == nullptr)return;
        acOper->setEmitter(senderObject);
        acOper->setArgs("objID", objID);
        acOper->actionTriggered();
    }

    void TreeWidget::addMenuActions(QMenu & menu, QString actions, QString objectName)
    {
        auto act = menu.addAction(objectName); // 添加动作
        act->setObjectName(actions);
        connect(act, SIGNAL(triggered()), this, SLOT(acitonClicked()));
    }
} // namespace GUIOper