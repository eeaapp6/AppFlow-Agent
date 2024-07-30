#include "OperGraphPreprocess.h"

// APP
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Graph widget and object
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObject3D.h"

// Graph data manager
#include "GraphDataProvider/GraphProviderManager.h"
#include "GraphDataProvider/GraphModelProvider.h"
#include "GraphDataProvider/GraphMarkProvider.h"

// Data
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"

// GUI
#include "GUIFrame/MainTreeWidget.h"

namespace GUIOper
{
    void OperGraphPreprocess::updateGraph(int dataObjId, bool forceUpdate)
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
        Exchange::FITKOCC2VTKGraphObject3D* obj{ nullptr };
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs;
        bool isValid = false;

        // 检查数据ID是否为模型。
        Interface::FITKAbsGeoCommand* model = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKAbsGeoCommand>(dataObjId);
        if (model && !isValid)
        {
            obj = modelProvider->getModelGraphObject(dataObjId);
            isValid = true;
        }

        // 检查数据ID是否为流体网格。
        Interface::FITKUnstructuredFluidMeshVTK* fluidMesh = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKUnstructuredFluidMeshVTK>(dataObjId);
        if (fluidMesh && !isValid)
        {
            objs = modelProvider->getFuildBoundMeshGraphObjects(dataObjId);
            isValid = true;
        }

        // 检查数据ID是否为流体域形状数据。
        Interface::FITKAbstractRegionMeshSize* regionMesh = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKAbstractRegionMeshSize>(dataObjId);
        if (regionMesh && !isValid)
        {
            obj = modelProvider->getRegionMeshGraphObject(dataObjId);
            isValid = true;
        }

        if (obj)
        {
            objs.push_back(obj);
        }

        // 添加至三维窗口。
        for (Exchange::FITKOCC2VTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            obj->update(forceUpdate);

            addGraphObjectToWidget(obj, graphWidget, false);
        }
    }

    void OperGraphPreprocess::updateGraphByType(int type, GraphOperParam param)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 获取符号可视化对象管理器。
        GraphData::GraphMarkProvider* markProvider = GraphData::GraphProviderManager::getInstance()->getMarkProvider(graphWidget);
        if (!markProvider)
        {
            return;
        }

        // 获取或创建可视化对象。（type数值与树形节点类型枚举对应。）
        Exchange::FITKOCC2VTKGraphObject3D* obj = markProvider->getGraphObjectByType(type);
        if (!obj)
        {
            return;
        }

        // 首先添加至三维窗口。
        addGraphObjectToWidget(obj, graphWidget, false);

        // 刷新数据。
        obj->update(param.ForceUpdate);

        // 高亮/取消高亮。
        switch (param.HighlightMode)
        {
        case HighlightLevel::DisHighlight:
        {
            obj->disHighlight();
            break;
        }
        case HighlightLevel::Highlight:
        {
            obj->highlight();
            break;
        }
        case HighlightLevel::AdvHighlight:
        {
            obj->advanceHighlight(ShapeType::ShapeTypeNone, param.AdvHighlightIndice);
            break;
        }
        default:
            break;
        }
        
        // 隐藏或显示。
        obj->setVisible(param.Visibility);
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

    void OperGraphPreprocess::setEnableModelTransparent(bool flag)
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

        // 启用或关闭半透明。
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs = modelProvider->getAllModelGraphObjects();
        for (Exchange::FITKOCC2VTKGraphObject3D* obj : objs)
        {
            if (obj)
            {
                obj->setTransparent(flag);
            }
        }
    }

    void OperGraphPreprocess::reRender(bool fitView)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        if (fitView)
        {
            graphWidget->fitView();
        }
        else
        {
            graphWidget->reRender();
        }
    }
}  // namespace GUIOper
