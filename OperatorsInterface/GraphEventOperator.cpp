#include "GraphEventOperator.h"

namespace EventOper
{
    void GraphEventOperator::updateGraph(int dataId, bool fitView)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataId);
        Q_UNUSED(fitView);
    }
}  // namespace EventOper
