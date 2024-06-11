/**
 * @file   RenderWidget.h
 * @brief  渲染区子面板
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-11
 */
#ifndef __RENDERWIDGET_H__
#define __RENDERWIDGET_H__

#include "PanelWidgetBase.h"

namespace Comp
{
	class FITKMdiArea;
}

namespace GUI
{
	/**
	 * @brief  渲染区子面板类
	 * @author YanZhiHui (chanyuantiandao@126.com)
	 * @date   2024-06-11
	 */
	class RenderWidget : public PanelWidgetBase
	{
		Q_OBJECT

	public:
		/**
		 * @brief  构造函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		RenderWidget(MainWindow *mainWindow);
		/**
		 * @brief  析构函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-11
		 */
		~RenderWidget() = default;

	private:
		Comp::FITKMdiArea* m_MdiArea{};

	};
}
#endif // !__RENDERWIDGET_H__
