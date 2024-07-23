#include "GraphModelProvider.h"

// Render VTK
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphRender.h"

// Graph
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObjectShape.h"
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

    QList<Exchange::FITKOCC2VTKGraphObjectShape*> GraphModelProvider::getCurrentGraphObjs()
    {
        // 当前所有模型可视化对象数据。
        QList<Exchange::FITKOCC2VTKGraphObjectShape*> objs;

        // 模型（几何）可视化对象。
        objs << m_modelObjHash.values();

        return objs;
    }

    Exchange::FITKOCC2VTKGraphObjectShape* GraphModelProvider::getModelGraphObject(int dataId)
    {
        // 检查数据ID。
        Exchange::FITKOCC2VTKGraphObjectShape* obj{ nullptr };

        // 检查数据ID。
        Interface::FITKAbstractModel* model = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKAbstractModel>(dataId);
        if (!model)
        {
            return obj;
        }

        // 创建过则返回。
        if (m_modelObjHash.contains(dataId))
        {
            return m_modelObjHash[dataId];
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
        m_modelObjHash.insert(dataId, obj);

        // 检测数据析构对三维数据进行析构并移出数据管理。
        //@{
        connect(model, &Interface::FITKAbstractModel::dataObjectDestoried, this, [=]
        {
            Core::FITKAbstractGraphObject* gObj = m_modelObjHash.take(dataId);
            if (gObj)
            {
                delete gObj;
            }
        });
        //@}

        return obj;
    }

    bool GraphModelProvider::updateObjById(int dataId, QVariant info)
    {
        // 参数预留。
        Q_UNUSED(info);

        if (!m_modelObjHash.contains(dataId))
        {
            return false;
        }

        // 获取可视化对象并更新。
        Core::FITKAbstractGraphObject* obj = m_modelObjHash[dataId];
        if (!obj)
        {
            return false;
        }

        obj->update();

        return true;
    }

    void GraphModelProvider::setVertPickable()
    {
        // 开启可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObjectShape*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObjectShape* obj : objs)
        {
            if (obj)
            {
                obj->setPickMode(ShapePickMode::PickVertex);
            }
        }
    }

    void GraphModelProvider::setEdgePickable()
    {
        // 开启可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObjectShape*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObjectShape* obj : objs)
        {
            if (obj)
            {
                obj->setPickMode(ShapePickMode::PickEdge);
            }
        }
    }

    void GraphModelProvider::setFacePickable()
    {
        // 开启可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObjectShape*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObjectShape* obj : objs)
        {
            if (obj)
            {
                obj->setPickMode(ShapePickMode::PickFace);
            }
        }
    }

    void GraphModelProvider::setSolidPickable()
    {
        // 开启可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObjectShape*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObjectShape* obj : objs)
        {
            if (obj)
            {
                obj->setPickMode(ShapePickMode::PickSolid);
            }
        }
    }

    void GraphModelProvider::setNonePickable()
    {
        // 关闭可拾取状态。
        QList<Exchange::FITKOCC2VTKGraphObjectShape*> objs = getCurrentGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObjectShape* obj : objs)
        {
            if (obj)
            {
                obj->setPickMode(ShapePickMode::PickNone);
            }
        }
    }
}   // namespace GraphData