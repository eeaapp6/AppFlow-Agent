/**
 * @file   RenderWidget.h
 * @brief  渲染区子面板
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-11
 */
#ifndef __RENDERWIDGET_H__
#define __RENDERWIDGET_H__

#include "GUIFrameAPI.h"
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
	class GUIFRAMEAPI RenderWidget : public PanelWidgetBase
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
		/**
		 * @brief  获取多视口管理
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		Comp::FITKMdiArea* getMdiArea() const;
	private:
		/**
		 * @brief  多视口管理
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date   2024-06-12
		 */
		Comp::FITKMdiArea* m_MdiArea{};

	};
}
#endif // !__RENDERWIDGET_H__
