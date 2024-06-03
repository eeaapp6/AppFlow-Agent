/**
 * @file ComponentFactory.cpp
 * @brief 组件工厂
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-04-19
 */
#include "ComponentFactory.h"
#include "FITK_Kernal/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernal/FITKAppFramework/FITKGlobalData.h"
#include "FITK_GeneralComponent/FITKCompMessageWidget/FITKConsoleComponent.h"
#include "FITK_GeneralComponent/FITKRenderWindowVTK/FITKGraph3DWindowInterface.h"
#include "FITK_GeneralComponent/FITKAbaqusIOINP/FITKAbaqusIOINPInterface.h"
#include "FITK_GeneralComponent/FITKCalculiXInpIO/FITKCalculiXINPIOInterface.h"


QList<AppFrame::FITKComponentInterface *> ComponentFactory::createComponents()
{
    // 自定义组件列表
    QList<AppFrame::FITKComponentInterface *> componentList;
    // 消息窗口组件
    componentList << new Comp::ConsoleComponent(FITKAPP->getGlobalData()->getMainWindow());
    // 3D图形窗口组件
    auto compVTKrender = new Comp::FITKGraph3DWindowInterface;
    componentList << compVTKrender;


 //   compVTKrender->addInitializer(1, new PreWindowInitializer);

    return componentList;
}
