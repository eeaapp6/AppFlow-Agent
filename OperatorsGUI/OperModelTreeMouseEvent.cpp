#include "OperModelTreeMouseEvent.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIFrame/MainTreeWidget.h"
#include "GUIWidget/TreeWidget.h"
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

namespace GUIOper
{
    // OperModelTreeMouseEvent类构造函数
    OperModelTreeMouseEvent::OperModelTreeMouseEvent(/* args */)
    {

    }
    // OperModelTreeMouseEvent类析构函数
    OperModelTreeMouseEvent::~OperModelTreeMouseEvent()
    {

    }
 
    // 更新所有模型案例树方法
    void OperModelTreeMouseEvent::updateTree()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        GUI::TreeWidget* treeWidget = mainWindow->getTreeWidget()->getTreeWidget();
        treeWidget->updateTree();
    }

    void OperModelTreeMouseEvent::moveProcessToStep(int step, void * addInfo)
    {
        if (step == 0) {
            GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
            mainWindow->getPropertyWidget()->init();
        }
    }
} 