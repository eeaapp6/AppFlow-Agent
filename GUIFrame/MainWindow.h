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

	/**
	 * @brief  主窗口界面类
	 * @author YanZhiHui (chanyuantiandao@126.com)
	 * @date   2024-06-12
	 */
	class GUIFRAMEAPI MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		/**
		 * @brief  构造函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		explicit MainWindow(QWidget *parent = nullptr);
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
         * @brief  获取菜单
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        MainMenuBase* getMainMenuBase() const;
        /**
         * @brief  获取控制面板子部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        ControlPanelWidget* getControlPanelWidget() const;
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
		 * @brief  初始化菜单（包括菜单栏和工具栏）
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		void initMenu();
		/**
		 * @brief  初始化中间部件
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		void initCentralWidget();
		/**
		 * @brief  初始化状态栏
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		void initStatusBar();

	private:
		/**
		 * @brief  Ui
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		Ui::MainWindow *m_Ui{};
		/**
		 * @brief  菜单
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		MainMenuBase* m_MainMenu{};
		/**
		 * @brief  控制面板
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		ControlPanelWidget* m_ControlWidget{};
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

	};
}

#endif // !__MAINWINDOW_H__
