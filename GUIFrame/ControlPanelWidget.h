/**
 * @file   ControlPanelWidget.h
 * @brief  控制面板子部件
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-11
 */
#ifndef __CONTROLPANELWIDGET_H__
#define __CONTROLPANELWIDGET_H__

#include "PanelWidgetBase.h"
 /**
  * @brief GUI命名空间
  * @since 1.0.0
  */
namespace GUI
{
	class MainWindow;
	/**
	 * @brief  控制面板子部件类
	 * @author YanZhiHui (chanyuantiandao@126.com)
	 * @date   2024-06-11
	 */
	class ControlPanelWidget : public PanelWidgetBase
	{
		Q_OBJECT
	public:
		/**
		 * @brief  构造函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		ControlPanelWidget(MainWindow *mainWindow);
		/**
		 * @brief  析构函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		~ControlPanelWidget();

	};
}
#endif // !__CONTROLPANELWINDOW_H__
