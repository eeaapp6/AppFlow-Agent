#include "GraphProviderBase.h"

// APP
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponentFactory.h"
#include "FITK_Kernel/FITKAppFramework/FITKAPPSettings.h"

// Render VTK
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphRender.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphObjectVTK.h"

// Render OCC
#include "FITK_Component/FITKRenderWindowOCC/FITKGraph3DWindowOCC.h"
#include "FITK_Component/FITKRenderWindowOCC/FITKGraphObjectOCC.h"

// Graph
#include "FITK_Component/FITKOCCGraphAdaptor/FITKGraphObjectShapeOCC.h"
#include "FITK_Component/FITKOCCGraphAdaptor/FITKGraphObjectShapeVTK.h"

// Graph widget
#include "FITK_Kernel/FITKCore/FITKAbstractGraphWidget.h"

namespace GraphData
{
    GraphProviderBase::GraphProviderBase(Core::FITKAbstractGraph3DWidget* graphWidget) :
        m_graphWidget(graphWidget)
    {
        // 获取三维可视化窗口引擎类型。
        if (!m_graphWidget)
        {
            return;
        }

        // VTK与OCC窗口检测。
        Comp::FITKGraph3DWindowVTK* vtkW = dynamic_cast<Comp::FITKGraph3DWindowVTK*>(m_graphWidget);
        Render::FITKGraph3DWindowOCC* occW = dynamic_cast<Render::FITKGraph3DWindowOCC*>(m_graphWidget);

        if (vtkW)
        {
            m_visualEngineName = "VTK";
        }

        if (occW)
        {
            m_visualEngineName = "OCC";
        }
    }

    GraphProviderBase::~GraphProviderBase()
    {
        // 析构三维可视化对象。
    }

    QList<Core::FITKAbstractGraphObject*> GraphProviderBase::getCurrentVisibleGraphObjs()
    {
        //获取所有可视化对象数据。
        QList<Core::FITKAbstractGraphObject*> objs = getCurrentGraphObjs();

        QList<Core::FITKAbstractGraphObject*> objsVisible;
        for (Core::FITKAbstractGraphObject* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getGraphWidget())
            {
                objsVisible.push_back(obj);
            }
        }

        // 子类重写。
        return objsVisible;
    }

    QString GraphProviderBase::getClassName()
    {
        return "GraphProviderBase";
    }

    // 批量析构Hash指针。
    void GraphProviderBase::deleteObjsHash(QHash<int, Core::FITKAbstractGraphObject*>& hash)
    {
        // 传入数据管理字典。
        for (Core::FITKAbstractGraphObject* obj : hash.values())
        {
            delete obj;
        }

        hash.clear();
    }

    // 批量析构双层Hash指针。
    void GraphProviderBase::deleteObjsHash(QHash<int, QHash<int, Core::FITKAbstractGraphObject*>>& hash)
    {
        // 传入数据管理字典。
        for (QHash<int, Core::FITKAbstractGraphObject* > subHash : hash.values())
        {
            for (Core::FITKAbstractGraphObject* obj : subHash.values())
            {
                if (obj)
                {
                    delete obj;
                }
            }

            subHash.clear();
        }

        hash.clear();
    }

    bool GraphProviderBase::updateObjById(int dataId, QVariant info)
    {
        Q_UNUSED(dataId);
        Q_UNUSED(info);
        return false;
    }

    void GraphProviderBase::updateVisibility()
    {

    }
}   // namespace GraphData