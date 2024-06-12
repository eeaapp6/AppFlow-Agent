#include "OperGraphPreprocess.h"

// APP
#include "FITK_Kernal/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernal/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernal/FITKAppFramework/FITKAPPSettings.h"

// Global data
#include "FITK_Kernal/FITKCore/FITKDataRepo.h"

// Graph widget and object
#include "FITK_Kernal/FITKCore/FITKAbstractGraphWidget.h"
#include "FITK_Kernal/FITKCore/FITKAbstractGraphObject.h"

// Graph data manager
#include "GraphDataProvider/GraphProviderManager.h"
#include "GraphDataProvider/GraphModelProvider.h"

// GUI

namespace GUIOper
{
    void OperGraphPreprocess::updateGraph(int dataId, bool fitView)
    {
        // 获取可视化窗口。
        Core::FITKAbstractGraph3DWidget* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 获取模型可视化对象管理器。
        GraphData::GraphModelProvider* modelProvider = GraphData::GraphProviderManager::getInstance()->getModelProvider(graphWidget);
        if (!modelProvider)
        {
            return;
        }

        // 获取或创建可视化对象。
        Core::FITKAbstractGraphObject* obj = modelProvider->getModelGraphObject(dataId);
        if (!obj)
        {
            return;
        }

        // 添加至三维窗口。
        addGraphObjectToWidget(obj, graphWidget, fitView);
    }
}  // namespace GUIOper