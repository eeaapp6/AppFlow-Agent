/*****************************************************************//**
 * @file    GraphModelProvider.h
 * @brief   三维模型可视化数据管理。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef __GRAPHMODELPROVIDER_H__
#define __GRAPHMODELPROVIDER_H__

#include "GraphProviderBase.h"

#include "GraphDataProviderAPI.h"

#include <QHash>

// 前置声明

namespace GraphData
{
    /**
     * @brief   三维可视化数据分类管理类。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class GRAPHDATAPROVIDERAPI GraphModelProvider : public GraphProviderBase
    {
        Q_OBJECT

        // 友元，防止外部手动创建或析构。
        friend class GraphProviderManager;

    public:
        /**
         * @brief   获取当前算例已实例化的可视化对象。
         * @return  可视化对象列表。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QList<Exchange::FITKOCC2VTKGraphObjectShape*> getCurrentGraphObjs() override;

        /**
         * @brief   获取类名。[重写]
         * @return  类名
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QString getClassName() override;

        /**
         * @brief   根据数据ID获取对应可视化对象。（没有则创建）
         * @param   dataId：数据ID
         * @return  可视化对象
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        Exchange::FITKOCC2VTKGraphObjectShape* getModelGraphObject(int dataId);

        /**
         * @brief   根据数据ID更新可视化对象，没有则跳出。
         * @param   dataId：数据ID
         * @param   info：附加信息[缺省]
         * @return  是否存在该数据ID所对应可视化对象。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        bool updateObjById(int dataId, QVariant info = QVariant());

    private:
        /**
         * @brief   构造函数。
         * @param   graphWidget：可视化窗口
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        GraphModelProvider(Comp::FITKGraph3DWindowVTK* graphWidget);

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        ~GraphModelProvider();

    private:
        /**
         * @brief   模型（几何）数据字典。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QHash<int, Exchange::FITKOCC2VTKGraphObjectShape*> m_modelObjHash;

    };
}   // namespace GraphData

#endif // __GRAPHMODELPROVIDER_H__
