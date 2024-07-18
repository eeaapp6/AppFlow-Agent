/**
 * @file   MainWindow.h
 * @brief  主窗口界面
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-12
 */
#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QMainWindow>
#include "GUIFrameAPI.h"

#include "FITK_Kernel/FITKCore/FITKAbstractGUIObject.h"
#include "Tools/Win64/SARibbon/include/SARibbon-2.0.1/SARibbonMainWindow.h"

#include <QFont>

class SARibbonBar;

namespace Ui {
	class MainWindow;
}

namespace GUI
{
	class MainMenuBase;
	class ControlPanelWidget;
	class RenderWidget;
	class PropertyWidget;
	class GroupPropertyWidget;
	class ActionEventHandler;
    class MainTreeWidget;

	/**
	 * @brief  主窗口界面类
	 * @author YanZhiHui (chanyuantiandao@126.com)
	 * @date   2024-06-12
	 */
	class GUIFRAMEAPI MainWindow : public SARibbonMainWindow, public Core::FITKAbstractGUIObject
	{
        Q_OBJECT;
	public:
		/**
		 * @brief  构造函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		MainWindow(QWidget *parent = nullptr);
		/**
		 * @brief  析构函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		~MainWindow();

		/**
		 * @brief  获取界面的action消息处理器
		 * @return 消息处理器
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @data   2024-06-12
		 */
		ActionEventHandler* getActionEventHandle() const;
        /**
         * @brief  获取渲染子部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        RenderWidget* getRenderWidget() const;
        /**
         * @brief  获取属性子部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        PropertyWidget* getPropertyWidget() const;
        /**
         * @brief  获取组属性子部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        GroupPropertyWidget* getGroupPropertyWidget() const;

	private:
		/**
		 * @brief  初始化
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		void init();
		/**
		 * @brief  初始化中间部件
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		void initCentralWidget();
        /**
         * @brief 模块初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-11
         */
        void initApplicationButton();
        void initHome();
        void initGeometry();
        void initMesh();
        void initSetting();
        void initResult();
        void initHelp();
	private:
        QWidget* _currentWidget = nullptr;
        /**
         * @brief RibbonBar对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        SARibbonBar* _ribbonBar = nullptr;
        /**
         * @brief 树结构界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        MainTreeWidget* _treeWidget = nullptr;
		/**
		 * @brief  渲染部件
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		RenderWidget* m_RenderWidget{};
		/**
		 * @brief  属性面板
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		PropertyWidget* m_PropertyWidget{};
		/**
		 * @brief  组属性面板
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		GroupPropertyWidget* m_GroupPropertyWidget{};
		/**
		 * @brief  action点击事件处理
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		ActionEventHandler* m_ActionHandler{};
        /**
         * @brief 字符格式
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        QFont _font = QFont("Arial", 9);

	};
}

#endif // !__MAINWINDOW_H__
