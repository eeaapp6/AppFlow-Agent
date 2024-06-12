/*****************************************************************//**
 * @file    GraphProviderBase.h
 * @brief   三维可视化数据管理基类。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef __GRAPHPROVIDERBASE_H__
#define __GRAPHPROVIDERBASE_H__

#include <QObject>

#include "GraphDataProviderAPI.h"

#include <QHash>

// 前置声明
namespace Core
{
    class FITKAbstractGraphWidget;
    class FITKAbstractGraphObject;
}

namespace Comp
{
    class FITKGraphObjectVTK;
}

namespace Render
{
    class FITKGraphObjectOCC;
}

namespace GraphData
{
    /**
     * @brief   三维可视化数据分类管理类。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class GRAPHDATAPROVIDERAPI GraphProviderBase : public QObject
    {
        Q_OBJECT

        // 友元，防止外部手动创建或析构。
        friend class GraphProviderManager;

    public:
        /**
         * @brief   获取当前算例已实例化的可视化对象。[纯虚函数]
         * @return  可视化对象列表。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual QList<Core::FITKAbstractGraphObject*> getCurrentGraphObjs() = 0;

        /**
         * @brief   获取当前算例已实例化的当前可见可视化对象。
         * @return  可视化对象列表。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QList<Core::FITKAbstractGraphObject*> getCurrentVisibleGraphObjs();

        /**
         * @brief   获取类名。[虚函数]
         * @return  类名
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual QString getClassName();

        /**
         * @brief   根据数据ID更新可视化对象，没有则跳出。
         * @param   dataId：数据ID
         * @param   info：附加信息[缺省]
         * @return  是否存在该数据ID所对应可视化对象。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        bool updateObjById(int dataId, QVariant info = QVariant());

        /**
         * @brief   更新全部当前可见可视化对象显示状态。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void updateVisibility();

    protected:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        GraphProviderBase();

        /**
         * @brief   析构函数。[虚函数]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual ~GraphProviderBase();

        /**
         * @brief   移除数据管理字典中的所有可视化对象数据并析构。
         * @param   hash：数据字典
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void deleteObjsHash(QHash<int, Core::FITKAbstractGraphObject*>& hash);

        /**
         * @brief   移除数据管理字典中的所有可视化对象数据并析构。
         * @param   hash：数据字典
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void deleteObjsHash(QHash<int, QHash<int, Core::FITKAbstractGraphObject*>>& hash);

    protected:
        /**
         * @brief   临时预览可视化对象数据字典。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QHash<int, Core::FITKAbstractGraphObject*> m_previewObjHash = {};

    };
}   // namespace GraphData

#endif // __GRAPHPROVIDERBASE_H__
