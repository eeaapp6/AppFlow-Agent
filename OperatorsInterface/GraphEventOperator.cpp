#include "GraphEventOperator.h"

namespace EventOper
{
    void GraphEventOperator::updateGraph(int dataObjId, bool forceUpdate)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataObjId);
        Q_UNUSED(forceUpdate);
    }

    Exchange::FITKOCC2VTKGraphObject3D* GraphEventOperator::getModelGraphObjectByDataId(int dataObjId)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataObjId);
        return nullptr;
    }

    void GraphEventOperator::reRender(bool fitView)
    {
        // 根据不同子类重写。
        Q_UNUSED(fitView);
    }
}  // namespace EventOper
