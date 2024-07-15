/**
 * @file ComponentFactory.cpp
 * @brief 组件工厂
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-04-19
 */
#include "ComponentFactory.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Component/FITKCompMessageWidget/FITKConsoleComponent.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowInterface.h"
#include "FITK_Component/FITKRenderWindowOCC/FITKGraph3DWindowOCCInterface.h"
#include "FITK_Component/FITKGeoCompOCC/FITKGeoCompOCCInterface.h"
#include "OperatorsModel/OpersModelInterface.h"
#include "OperatorsGUI/OpersGUIInterface.h"
#include "PreWindowInitializer.h"

QList<AppFrame::FITKComponentInterface *> ComponentFactory::createComponents()
{
	// 自定义组件列表
	QList<AppFrame::FITKComponentInterface *> componentList;
	// 消息窗口组件
	componentList << new Comp::ConsoleComponent(FITKAPP->getGlobalData()->getMainWindow());
	// 3D图形窗口组件
	auto compVTKrender = new Comp::FITKGraph3DWindowInterface;
	componentList << compVTKrender;
    // OCC图形窗口组件
    componentList << new Render::FITKGraph3DWindowOCCInterface;
    //occ 建模
    componentList << new OCC::FITKGeoCompOCCInterface;
	// 模型数据控制器组件
	componentList << new OperModel::OpersModelInterface;
	// 界面控制器组件
	componentList << new GUIOper::OperatorsGUIInterface;

    // VTK窗口初始化器
    compVTKrender->addInitializer(1, new PreWindowInitializer);

	return componentList;
}
