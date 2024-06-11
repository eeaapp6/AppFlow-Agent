/**
 * @file   MenuWidget.h
 * @brief  主窗口菜单
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-11
 */
#ifndef __MAINMENU_H__
#define __MAINMENU_H__

#include "GUIWidget/MainMenuBase.h"
#include <QMenuBar>
#include <QToolBar>

namespace GUI
{
	/**
	 * @brief  主窗口菜单类
	 * @author YanZhiHui (chanyuantiandao@126.com)
	 * @date   2024-06-11
	 */
	class MainMenu : public MainMenuBase
	{
		Q_OBJECT

	public:
		/**
		 * @brief  构造函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		MainMenu(MainWindow *mainWindow);
		/**
		 * @brief  析构函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		~MainMenu() = default;
		/**
		 * @brief  添加菜单
		 * @param  title 菜单标题
		 * @param  actions actions列表
		 * @param  parent 父级菜单
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		void addMenu(QString title, QList<MenuActionItem*> actions, QMenu* parent) override;
		/**
		 * @brief  添加工具条菜单
		 * @param  title 菜单标题
		 * @param  actions actions列表
		 * @param  style 显示风格
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		void addToolMenu(QString title, QList<MenuActionItem*> actions, Qt::ToolButtonStyle style = Qt::ToolButtonStyle::ToolButtonIconOnly) override;

	protected:
		QMenuBar* m_MenuBar{};

		QHash<QString, QToolBar*> m_MenuToolBars{};

		QHash<QString, QActionGroup*> m_ActionGroups{};


	};
}
#endif // !__MAINMENU_H__
