#include "GraphEventOperator.h"

namespace EventOper
{
    void GraphEventOperator::updateGraph(int dataObjId, bool fitView)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataObjId);
        Q_UNUSED(fitView);
    }

    Exchange::FITKOCC2VTKGraphObjectShape* GraphEventOperator::getModelGraphObjectByDataId(int dataObjId)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataObjId);
        return nullptr;
    }
}  // namespace EventOper
