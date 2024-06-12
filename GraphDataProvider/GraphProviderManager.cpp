#include "GraphProviderManager.h"

// Graph widget
#include "FITK_Kernal/FITKCore/FITKAbstractGraphWidget.h"

// Provider
#include "GraphModelProvider.h"

namespace GraphData
{
    // 静态变量初始化
    GraphProviderManager* GraphProviderManager::s_instance = nullptr;
    QMutex GraphProviderManager::m_mutex;

    GraphProviderManager* GraphProviderManager::getInstance()
    {
        // 获取单例实例。
        if (!s_instance)
        {
            s_instance = new GraphProviderManager;
        }

        return s_instance;
    }

    void GraphProviderManager::Delete()
    {
        // 手动销毁实例。
        if (s_instance)
        {
            delete s_instance;
        }

        s_instance = nullptr;
    }

    GraphProviderManager::GraphProviderManager()
    {
        // 初始化预览数据管理器。
        m_previewModelProvider = new GraphModelProvider;
    }

    GraphProviderManager::~GraphProviderManager()
    {
        // 清除数据管理器。
        deleteProvider(m_modelProvider);

        // 清除预览数据管理器。
        deleteProvider(m_previewModelProvider);
    }

    GraphModelProvider* GraphProviderManager::getPreviewModelProvider()
    {
        return m_previewModelProvider;
    }

    GraphModelProvider* GraphProviderManager::getModelProvider()
    {
        // 不存在则创建新管理器。
        if (!m_modelProvider)
        {
            m_modelProvider = new GraphModelProvider;
        }
      
        return m_modelProvider;
    }

    void GraphProviderManager::updateObjectById(int dataId)
    {
        if (dataId == -1 || !m_modelProvider)
        {
            return;
        }

        bool flag = m_modelProvider->updateObjById(dataId);
        Q_UNUSED(flag);
    }
}   // namespace GraphData