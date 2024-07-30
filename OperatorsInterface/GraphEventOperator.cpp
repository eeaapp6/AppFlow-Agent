#include "GraphEventOperator.h"

namespace EventOper
{
    void GraphEventOperator::updateGraph(int dataObjId, bool forceUpdate)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataObjId);
        Q_UNUSED(forceUpdate);
    }

    void GraphEventOperator::updateGraphByType(int type, GraphOperParam param)
    {
        // 根据不同子类重写。
        Q_UNUSED(type);
        Q_UNUSED(param);
    }

    Exchange::FITKOCC2VTKGraphObject3D* GraphEventOperator::getModelGraphObjectByDataId(int dataObjId)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataObjId);
        return nullptr;
    }

    void GraphEventOperator::setEnableModelTransparent(bool isOn)
    {
        // 根据不同子类重写。
        Q_UNUSED(isOn);
    }

    void GraphEventOperator::highlight(int dataObjId, QVariant info, QColor color)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataObjId);
        Q_UNUSED(info);
        Q_UNUSED(color);
    }

    void GraphEventOperator::advHighlight(int dataObjId, QVector<int> & indice, QColor color)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataObjId);
        Q_UNUSED(indice);
        Q_UNUSED(color);
    }

    void GraphEventOperator::clearHighlight()
    {
        // 根据不同子类重写。
    }

    void GraphEventOperator::reRender(bool fitView)
    {
        // 根据不同子类重写。
        Q_UNUSED(fitView);
    }
}  // namespace EventOper
