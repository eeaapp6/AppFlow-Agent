#include "OperGraphPreprocess.h"

// APP
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Graph widget and object
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphObjectVTK.h"

// Graph data manager
#include "GraphDataProvider/GraphProviderManager.h"
#include "GraphDataProvider/GraphModelProvider.h"

// GUI

namespace GUIOper
{
    void OperGraphPreprocess::updateGraph(int dataObjId, bool fitView)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
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
        Exchange::FITKOCC2VTKGraphObject3D* obj = modelProvider->getModelGraphObject(dataObjId);
        if (!obj)
        {
            return;
        }

        // 添加至三维窗口。
        addGraphObjectToWidget(obj, graphWidget, fitView);
    }

    Exchange::FITKOCC2VTKGraphObject3D* OperGraphPreprocess::getModelGraphObjectByDataId(int dataObjId)
    {
        // 可视化对象。
        Exchange::FITKOCC2VTKGraphObject3D* obj{ nullptr };
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return obj;
        }

        // 获取模型可视化对象管理器。
        GraphData::GraphModelProvider* modelProvider = GraphData::GraphProviderManager::getInstance()->getModelProvider(graphWidget);
        if (!modelProvider)
        {
            return obj;
        }

        obj = modelProvider->getCurrentGraphObjByDataId(dataObjId);
        return obj;
    }
}  // namespace GUIOper
