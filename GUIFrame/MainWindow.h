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

	class GUIFRAMEAPI MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = nullptr);
		~MainWindow();

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

	};
}

#endif // !__MAINWINDOW_H__
