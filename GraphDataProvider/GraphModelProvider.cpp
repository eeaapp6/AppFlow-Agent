#include "GraphModelProvider.h"

// Render VTK
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphRender.h"

// Graph
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObject3D.h"
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObjectModel.h"

// Adaptor
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKViewAdaptorBase.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Data
#include "FITK_Component/FITKGeoCompOCC/FITKAbstractOCCModel.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFulidMeshVTK.h"

// Graph widget
#include "FITK_Kernel/FITKCore/FITKAbstractGraphWidget.h"

namespace GraphData
{
    GraphModelProvider::GraphModelProvider(Comp::FITKGraph3DWindowVTK* graphWidget)
        : GraphProviderBase(graphWidget)
    {

    }

    GraphModelProvider::~GraphModelProvider()
    {
        // 析构三维可视化对象。
        deleteObjsHash(m_modelObjHash);
        deleteObjsHash(m_boundMeshObjHash);
    }

    QString GraphModelProvider::getClassName()
    {
        return "GraphModelProvider";
    }

    QList<Exchange::FITKOCC2VTKGraphObject3D*> GraphModelProvider::getCurrentGraphObjs()
    {
        // 当前所有模型可视化对象数据。
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs;

        // 模型（几何）可视化对象。
        objs << m_modelObjHash.values();

        // 边界网格可视化对象。
        objs << m_boundMeshObjHash.values();

        return objs;
    }

    Exchange::FITKOCC2VTKGraphObject3D* GraphModelProvider::getModelGraphObject(int dataObjId)
    {
        // 模型可视化对象。
        Exchange::FITKOCC2VTKGraphObject3D* obj{ nullptr };

        // 检查数据ID。
        Interface::FITKAbstractModel* model = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKAbstractModel>(dataObjId);
        if (!model)
        {
            return obj;
        }

        // 模型可视化对象。
        obj = getGraphObject("ModelOCC", m_modelObjHash, model);

        return obj;
    }

    Exchange::FITKOCC2VTKGraphObject3D* GraphModelProvider::getBoundMeshGraphObject(int dataObjId)
    {
        // 边界网格可视化对象。
        Exchange::FITKOCC2VTKGraphObject3D* obj{ nullptr };

        // 检查数据ID。
        Interface::FITKBoundaryMeshVTK* boundMesh = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKBoundaryMeshVTK>(dataObjId);
        if (!boundMesh)
        {
            return obj;
        }

        // 模型可视化对象。
        obj = getGraphObject("BoundMesh", m_boundMeshObjHash, boundMesh);

        return obj;
    }

    QList<Exchange::FITKOCC2VTKGraphObject3D*> GraphModelProvider::getFuildBoundMeshGraphObjects(int dataObjId)
    {
        // 模型可视化对象。
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs;

        // 检查数据ID。
        Interface::FITKUnstructuredFluidMeshVTK* fluidMesh = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKUnstructuredFluidMeshVTK>(dataObjId);
        if (!fluidMesh)
        {
            return objs;
        }

        // 获取边界网格管理器。
        Interface::FITKBoundaryMeshVTKManager* bdMeshMgr = fluidMesh->getBoundaryMeshManager();
        if (!bdMeshMgr)
        {
            return objs;
        }

        // 生成可视化对象。
        int nBdMesh = bdMeshMgr->getDataCount();

        for (int i = 0; i < nBdMesh; i++)
        {
            Interface::FITKBoundaryMeshVTK* bdMesh = bdMeshMgr->getDataByIndex(i);
            if (!bdMesh)
            {
                continue;
            }

            int bdMeshId = bdMesh->getDataObjectID();
            Exchange::FITKOCC2VTKGraphObject3D* obj = getBoundMeshGraphObject(bdMeshId);
            if (obj)
            {
                objs.push_back(obj);
            }
        }

        return objs;
    }

    Exchange::FITKOCC2VTKGraphObject3D* GraphModelProvider::getCurrentGraphObjByDataId(int dataObjId)
    {
        // 查找模型。
        if (m_modelObjHash.contains(dataObjId))
        {
            return m_modelObjHash[dataObjId];
        }

        return nullptr;
    }

    bool GraphModelProvider::updateObjById(int dataObjId, QVariant info)
    {
        // 参数预留。
        Q_UNUSED(info);

        if (!m_modelObjHash.contains(dataObjId))
        {
            return false;
        }

        // 获取可视化对象并更新。
        Core::FITKAbstractGraphObject* obj = m_modelObjHash[dataObjId];
        if (!obj)
        {
            return false;
        }

        obj->update();

        return true;
    }

    void GraphModelProvider::setVertPickable(int dataObjId)
    {
        // 开启可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                obj->setPickMode(ShapePickMode::PickNone);
            }
            else
            {
                obj->setPickMode(ShapePickMode::PickVertex);
            }
        }
    }

    void GraphModelProvider::setEdgePickable(int dataObjId)
    {
        // 开启可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                obj->setPickMode(ShapePickMode::PickNone);
            }
            else
            {
                obj->setPickMode(ShapePickMode::PickEdge);
            }
        }
    }

    void GraphModelProvider::setFacePickable(int dataObjId)
    {
        // 开启可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                obj->setPickMode(ShapePickMode::PickNone);
            }
            else
            {
                obj->setPickMode(ShapePickMode::PickFace);
            }
        }
    }

    void GraphModelProvider::setSolidPickable(int dataObjId)
    {
        // 开启可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                obj->setPickMode(ShapePickMode::PickNone);
            }
            else
            {
                obj->setPickMode(ShapePickMode::PickSolid);
            }
        }
    }

    void GraphModelProvider::setNonePickable(int dataObjId)
    {
        // 关闭可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObject3D*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                // 其他数据不取消拾取。
                continue;
            }
            else
            {
                obj->setPickMode(ShapePickMode::PickNone);
            }
        }
    }
}   // namespace GraphData