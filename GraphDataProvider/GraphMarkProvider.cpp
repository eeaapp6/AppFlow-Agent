#include "GraphMarkProvider.h"

// Render VTK
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphRender.h"

// Graph
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObject3D.h"
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObjectMaterialPoints.h"

// Adaptor
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKViewAdaptorBase.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Data
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKZonePoints.h"

// Graph widget
#include "FITK_Kernel/FITKCore/FITKAbstractGraphWidget.h"

// GUI
#include "GUIFrame/MainTreeWidget.h"

namespace GraphData
{
    GraphMarkProvider::GraphMarkProvider(Comp::FITKGraph3DWindowVTK* graphWidget)
        : GraphProviderBase(graphWidget)
    {

    }

    GraphMarkProvider::~GraphMarkProvider()
    {
        // 析构三维可视化对象。
        deleteObjsHash(m_tempTypeObjHash);
    }

    QString GraphMarkProvider::getClassName()
    {
        return "GraphMarkProvider";
    }

    QList<Exchange::FITKOCC2VTKGraphObject3D*> GraphMarkProvider::getCurrentGraphObjs()
    {
        // 当前所有模型可视化对象数据。
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs;

        // 临时标识符可视化对象。
        // objs << m_tempTypeObjHash.values();

        return objs;
    }

    Exchange::FITKOCC2VTKGraphObject3D* GraphMarkProvider::getGraphObjectByType(int type)
    {
        // 符号可视化对象。
        Exchange::FITKOCC2VTKGraphObject3D* obj{ nullptr };

        if (m_tempTypeObjHash.contains(type))
        {
            return m_tempTypeObjHash[type];
        }

        // type数值与树形节点类型枚举对应。
        switch (type)
        {
        // 材料点。
        case int(GUI::MainTreeEnum::MainTree_MeshPoint):
        {
            // 获取材料点管理器。
            Interface::FITKMeshGenInterface* interMeshGen = Interface::FITKMeshGenInterface::getInstance();
            Interface::FITKZonePointManager* mPtsMgr = interMeshGen->getZonePointManager();
            if (mPtsMgr)
            {
                obj = new Exchange::FITKOCC2VTKGraphObjectMaterialPoints(mPtsMgr);
            }

            break;
        }
        default:
            break;
        }

        return obj;
    }
}   // namespace GraphData