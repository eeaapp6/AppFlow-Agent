#include "OperModelTreeMouseEvent.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIFrame/MainTreeWidget.h"
#include "OperatorsInterface/GraphEventOperator.h"

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

Q_DECLARE_METATYPE(GUI::MainTreeEnum)

namespace GUIOper
{
    // OperModelTreeMouseEvent类构造函数
    OperModelTreeMouseEvent::OperModelTreeMouseEvent(/* args */)
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow) {
            _treeWidget = mainWindow->getTreeWidget()->getTreeWidget();
            connect(_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onItemClicked(QTreeWidgetItem*, int)));
        }
    }
    // OperModelTreeMouseEvent类析构函数
    OperModelTreeMouseEvent::~OperModelTreeMouseEvent()
    {

    }
 
    // 更新所有模型案例树方法
    void OperModelTreeMouseEvent::updateTree()
    {
        if (_treeWidget == nullptr)return;
        _treeWidget->clear();

        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        QTreeWidgetItem* geometryItem = new QTreeWidgetItem();
        geometryItem->setText(0, "geometry");
        geometryItem->setExpanded(true);
        _treeWidget->addTopLevelItem(geometryItem);

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
            case Interface::FITKGeoEnum::FGTBox:  treeType = GUI::MainTreeEnum::MainTree_GeometyItem; break;
            case Interface::FITKGeoEnum::FGTCylinder:treeType = GUI::MainTreeEnum::MainTree_GeometyItem; break;
            case Interface::FITKGeoEnum::FGTSphere:treeType = GUI::MainTreeEnum::MainTree_GeometyItem;  break;
            }
            item->setData(2, 0, QVariant::fromValue(treeType));
            item->setData(3, 0, geometryType);

            geometryItem->addChild(item);
        }
    }

    void OperModelTreeMouseEvent::onItemClicked(QTreeWidgetItem * item, int column)
    {
        if (item == nullptr)return;
        int objID = item->data(1, 0).toInt();
        GUI::MainTreeEnum treeType = item->data(2, 0).value<GUI::MainTreeEnum>();
        Interface::FITKGeoEnum::FITKGeometryComType geometryType = item->data(3, 0).value<Interface::FITKGeoEnum::FITKGeometryComType>();

        QString name = "";
        switch (treeType){
        case GUI::MainTreeEnum::MainTree_Geomety: break;
        case GUI::MainTreeEnum::MainTree_GeometyItem: 
            switch (geometryType) {
            case Interface::FITKGeoEnum::FGTBox:name = "actionEditCube"; break;
            case Interface::FITKGeoEnum::FGTCylinder:name = "actionEditCylinder"; break;
            case Interface::FITKGeoEnum::FGTSphere:name = "actionEditSphere"; break;
            }
            break;
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
            GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
            GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
            if (propertyWidget) {
                propertyWidget->init();
            }
        }
    }

    void OperModelTreeMouseEvent::onDoubleClicked(QTreeWidgetItem * item, int column)
    {
        if (item == nullptr)return;
        int objID = item->data(1, 0).toInt();
        GUI::MainTreeEnum treeType = item->data(2, 0).value<GUI::MainTreeEnum>();
        Interface::FITKGeoEnum::FITKGeometryComType geometryType = item->data(3, 0).value<Interface::FITKGeoEnum::FITKGeometryComType>();
    }
} // namespace GUIOper