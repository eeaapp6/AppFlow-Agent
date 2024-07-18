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

// Graph
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObjectShape.h"

// Graph widget
#include "FITK_Kernel/FITKCore/FITKAbstractGraphWidget.h"

namespace GraphData
{
    GraphProviderBase::GraphProviderBase(Comp::FITKGraph3DWindowVTK* graphWidget) :
        m_graphWidget(graphWidget)
    {
        // 获取三维可视化窗口引擎类型。
        if (!m_graphWidget)
        {
            return;
        }
    }

    GraphProviderBase::~GraphProviderBase()
    {
        // 析构三维可视化对象。
    }

    QList<Exchange::FITKOCC2VTKGraphObjectShape*> GraphProviderBase::getCurrentVisibleGraphObjs()
    {
        //获取所有可视化对象数据。
        QList<Exchange::FITKOCC2VTKGraphObjectShape*> objs = getCurrentGraphObjs();

        QList<Exchange::FITKOCC2VTKGraphObjectShape*> objsVisible;
        for (Exchange::FITKOCC2VTKGraphObjectShape* obj : objs)
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
    void GraphProviderBase::deleteObjsHash(QHash<int, Exchange::FITKOCC2VTKGraphObjectShape*>& hash)
    {
        // 传入数据管理字典。
        for (Exchange::FITKOCC2VTKGraphObjectShape* obj : hash.values())
        {
            delete obj;
        }

        hash.clear();
    }

    // 批量析构双层Hash指针。
    void GraphProviderBase::deleteObjsHash(QHash<int, QHash<int, Exchange::FITKOCC2VTKGraphObjectShape*>>& hash)
    {
        // 传入数据管理字典。
        for (QHash<int, Exchange::FITKOCC2VTKGraphObjectShape* > subHash : hash.values())
        {
            for (Exchange::FITKOCC2VTKGraphObjectShape* obj : subHash.values())
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