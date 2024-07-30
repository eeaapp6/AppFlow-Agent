/*****************************************************************//**
 * @file    GraphEventOperator.h
 * @brief   三维可视对象数据操作器接口类。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef __GRAPHEVENTOPERATOR_H__
#define __GRAPHEVENTOPERATOR_H__

#include "FITK_Kernel/FITKCore/FITKAbstractOperator.h"

#include "OperatorsInterfaceAPI.h"

#include <QColor>
#include <QVariant>

// 前置声明
namespace Exchange
{
    class FITKOCC2VTKGraphObject3D;
}

enum HighlightLevel
{
    // 取消高亮。
    DisHighlight = 0,

    // 高亮。
    Highlight,

    // 高级高亮
    AdvHighlight
};

/**
 * @brief   可视化对象操作执行参数结构体。
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-07-26
 */
struct GraphOperParam
{
    // 操作的数据ID。（不存在则传入-1）
    int DataId = -1;

    // 隐藏与显示。
    bool Visibility = true;

    // 强制刷新。
    bool ForceUpdate = false;

    // 高亮级别。
    HighlightLevel HighlightMode = DisHighlight;

    // 高级高亮索引或ID列表。（高级高亮等功能使用）
    QVector<int> AdvHighlightIndice;

    // 高亮颜色。（缺省）
    QColor HighlightColor;
};

namespace EventOper
{
    /**
     * @brief   三维可视对象数据操作器接口类。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class OperatorsInterfaceAPI GraphEventOperator : public Core::FITKAbstractOperator
    {
        Q_OBJECT

    public:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        explicit GraphEventOperator() = default;

        /**
         * @brief   析构函数。[虚函数]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual ~GraphEventOperator() = default;

        // 前处理渲染与窗口功能。
        //@{
        /**
         * @brief   根据提供的数据对象ID更新对应可视化对象。（没有则创建）[虚函数]
         * @param   dataObjId：数据对象ID
         * @param   forceUpdate：是否强制刷新[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual void updateGraph(int dataObjId, bool forceUpdate = false);

        /**
         * @brief   根据提供的数据类型创建可视化对象并执行相关逻辑。（没有则创建）[虚函数]
         *          （此接口用于非数据类可视化对象预览。）
         * @param   type：数据类型
         * @param   param：可视化对象执行操作参数（默认只进行可视化）[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-26
         */
        virtual void updateGraphByType(int type, GraphOperParam param = GraphOperParam());

        /**
         * @brief   根据数据对象ID获取模型可视化对象。（没有则不创建）[虚函数]
         * @param   dataObjId：数据对象ID
         * @return  可视化对象
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-23
         */
        virtual Exchange::FITKOCC2VTKGraphObject3D* getModelGraphObjectByDataId(int dataObjId);
        //@}

        // 高亮功能接口。
        //@{
        
        //@}

        // 渲染专用接口。
        //@{
        /**
         * @brief   刷新渲染窗口。[虚函数]
         * @param   fitView：是否重置相机
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-24
         */
        virtual void reRender(bool fitView = false);
        //@}

        // 工具方法。
        //@{

        //@}

    };
}   // namespace EventOper

#endif // __GRAPHEVENTOPERATOR_H__