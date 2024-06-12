/**
 * @file   PropertyEmptyWidget.h
 * @brief  属性面板空子部件
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-12
 */
#ifndef __PROPERTYEMPTYWIDGET_H__
#define __PROPERTYEMPTYWIDGET_H__

#include "GUIWidgetAPI.h"
#include "PropertyChildWidgetBase.h"

namespace Ui
{
	class PropertyEmptyWidget;
}

namespace GUI
{
	class MainWindow;
	/**
	 * @brief  属性面板空子部件类
	 * @author YanZhiHui (chanyuantiandao@126.com)
	 * @date   2024-06-12
	 */
	class GUIWIDGETAPI PropertyEmptyWidget : public PropertyChildWidgetBase
	{
		Q_OBJECT
	public:
		/**
		 * @brief  构造函数
		 * @param  mainWindow 主窗口
		 * @param  tip 提示信息
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		PropertyEmptyWidget(MainWindow* mainWindow, QString tip = "");
		/**
		 * @brief  析构函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		virtual ~PropertyEmptyWidget();
		/**
		 * @brief  设置提示信息
		 * @param  info 提示信息
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		void setTip(QString info);

	private:
		/**
		 * @brief  UI
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		Ui::PropertyEmptyWidget* m_Ui{};
	};
}

#endif // !__PROPERTYEMPTYWIDGET_H__

