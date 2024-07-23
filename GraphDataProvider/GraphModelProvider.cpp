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

        return objs;
    }

    Exchange::FITKOCC2VTKGraphObject3D* GraphModelProvider::getModelGraphObject(int dataObjId)
    {
        // 检查数据ID。
        Exchange::FITKOCC2VTKGraphObject3D* obj{ nullptr };

        // 检查数据ID。
        Interface::FITKAbstractModel* model = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKAbstractModel>(dataObjId);
        if (!model)
        {
            return obj;
        }

        // 创建过则返回。
        if (m_modelObjHash.contains(dataObjId))
        {
            return m_modelObjHash[dataObjId];
        }

        // 生成可视化对象。
        Exchange::FITKOCC2VTKViewAdaptorBase* adaptor = FITKVIEWADAPTORFACTORY->createT<Exchange::FITKOCC2VTKViewAdaptorBase>("ModelOCC", model);
        if (!adaptor)
        {
            return obj;
        }

        adaptor->setDataObject(model);
        adaptor->update();

        obj = adaptor->getOutputData();

        // 适配器析构。
        delete adaptor;

        if (!obj)
        {
            return obj;
        }

        // 存储数据。
        m_modelObjHash.insert(dataObjId, obj);

        // 检测数据析构对三维数据进行析构并移出数据管理。
        //@{
        connect(model, &Interface::FITKAbstractModel::dataObjectDestoried, this, [=]
        {
            Core::FITKAbstractGraphObject* gObj = m_modelObjHash.take(dataObjId);
            if (gObj)
            {
                delete gObj;
            }
        });
        //@}

        return obj;
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
            if (obj)
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

            if (obj->getDataId() == dataObjId)
            {
                obj->setPickMode(ShapePickMode::PickEdge);
            }
            else
            {
                obj->setPickMode(ShapePickMode::PickNone);
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

            if (obj->getDataId() == dataObjId)
            {
                obj->setPickMode(ShapePickMode::PickFace);
            }
            else
            {
                obj->setPickMode(ShapePickMode::PickNone);
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

            if (obj->getDataId() == dataObjId)
            {
                obj->setPickMode(ShapePickMode::PickSolid);
            }
            else
            {
                obj->setPickMode(ShapePickMode::PickNone);
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

            if (obj->getDataId() == dataObjId)
            {
                obj->setPickMode(ShapePickMode::PickNone);
            }
            else
            {
                // 其他数据不取消拾取。
                continue;
            }
        }
    }
}   // namespace GraphData