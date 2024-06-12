#include "GraphEventOperator.h"

namespace EventOper
{
    void GraphEventOperator::updateGraph(int dataId)
    {
        // 根据不同子类重写。
        Q_UNUSED(dataId);
    }
}  // namespace EventOper
