/*****************************************************************//**
 * @file    GraphProviderManager.h
 * @brief   三维数据管理类。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef __GRAPHPROVIDERMANAGER_H__
#define __GRAPHPROVIDERMANAGER_H__

#include <QObject>

#include "GraphDataProviderAPI.h"

#include <QMuteX>
#include <QMutexLocker>
#include <QHash>

#include "FITK_Kernal/FITKCore/FITKCoreMacros.h"

// 前置声明
namespace Core
{
    class FITKAbstractGraph3DWidget;
}

namespace GraphData
{
    class GraphProviderBase;
    class GraphModelProvider;
}

namespace GraphData
{
    /**
     * @brief   管理所有三维数据分装类。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-03-27
     */
    class GRAPHDATAPROVIDERAPI GraphProviderManager : public QObject
    {
        Q_OBJECT
    public:
        /**
         * @brief   获取单例实例。[静态]
         * @return  实例
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        static GraphProviderManager* getInstance();

        /**
         * @brief   手动销毁。[静态]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        static void Delete();

        /**
         * @brief   获取三维模型数据分装类，没有则初始化。
         * @param   graphWidget：可视化窗口
         * @return  三维模型数据分装类指针
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        GraphModelProvider* getModelProvider(Core::FITKAbstractGraph3DWidget* graphWidget);

        /**
         * @brief   在所有算例数据管理尝试寻找该数据ID对应的可视化对象并刷新，如果不存在则跳出。
         * @param   dataId：需要更新的数据ID
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void updateObjectById(int dataId);

    private:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        GraphProviderManager();

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        ~GraphProviderManager();

        /**
         * @brief   析构数据管理器。
         * @param   provider：数据管理器[引用]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        template<class T>
        void deleteProvider(T*& provider)
        {
            if (provider)
            {
                delete provider;
            }

            provider = nullptr;
        }

        /**
         * @brief   批量析构数据管理器。
         * @param   providers：数据管理器列表[引用]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        template<class T>
        void deleteProviders(QList<T*>& providers)
        {
            // 清除数据管理指针。
            for (T* obj : providers)
            {
                if (obj)
                {
                    delete obj;
                }
            }

            providers.clear();
        }

    private:
        /**
         * @brief   静态实例。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        static GraphProviderManager* s_instance;

        /**
         * @brief   静态锁。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        static QMutex m_mutex;

        /**
         * @brief   三维窗口中算例的三维模型数据分装类。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        GraphModelProvider* m_modelProvider;

    };
}   // namespace GraphData

#endif // __GRAPHPROVIDERMANAGER_H__
