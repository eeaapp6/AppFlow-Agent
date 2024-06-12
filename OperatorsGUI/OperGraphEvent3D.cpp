#include "OperGraphEvent3D.h"

// App
#include "FITK_Kernal/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernal/FITKAppFramework/FITKGlobalData.h"

// Graph
#include "FITK_GeneralComponent/FITKOCCGraphAdaptor/FITKGraphObjectShapeOCC.h"
#include "FITK_GeneralComponent/FITKOCCGraphAdaptor/FITKGraphObjectShapeVTK.h"

// Render VTK
#include "FITK_GeneralComponent/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_GeneralComponent/FITKRenderWindowVTK/FITKGraphRender.h"
#include "FITK_GeneralComponent/FITKRenderWindowVTK/FITKGraphObjectVTK.h"

// Render OCC
#include "FITK_GeneralComponent/FITKRenderWindowOCC/FITKGraph3DWindowOCC.h"

// GUI
#include "GUIFrame/MainWindow.h"
#include "GUIFrame/RenderWidget.h"
#include "FITK_GeneralComponent/FITKWidget/FITKMdiArea.h"

namespace GUIOper
{
    Core::FITKAbstractGraph3DWidget* OperGraphEvent3D::getGraphWidget()
    {
        // 获取主窗口。
        GUI::MainWindow* mainWindow = FITKAPP->getGlobalData()->getMainWindowT<GUI::MainWindow>();
        if (!mainWindow)
        {
            return nullptr;
        }

        // 获取可视化区。
        GUI::RenderWidget* renderWidget{ nullptr };// = mainWindow->getRenderWidget();
        if (!renderWidget)
        {
            return nullptr;
        }

        Comp::FITKMdiArea* mdiArea{ nullptr };// = renderWidget->getMdiArea();;
        if (!mdiArea)
        {
            return nullptr;
        }

        // 获取当前窗口，尝试转换为三维窗口。
        QWidget* w = mdiArea->getCurrentWidget();
        Core::FITKAbstractGraph3DWidget* graphWidget = dynamic_cast<Core::FITKAbstractGraph3DWidget*>(w);

        return graphWidget;
    }

    void OperGraphEvent3D::addGraphObjectToWidget(Core::FITKAbstractGraphObject* obj, Core::FITKAbstractGraph3DWidget* graphWidget, bool fitView)
    {
        if (!obj || !graphWidget)
        {
            return;
        }

        // 检查窗口可视化引擎。
        Comp::FITKGraph3DWindowVTK* vtkW = dynamic_cast<Comp::FITKGraph3DWindowVTK*>(graphWidget);
        Render::FITKGraph3DWindowOCC* occW = dynamic_cast<Render::FITKGraph3DWindowOCC*>(graphWidget);

        // 转换可视化对象类型。
        Exchange::FITKGraphObjectShapeVTK* vtkObj = dynamic_cast<Exchange::FITKGraphObjectShapeVTK*>(obj);
        Exchange::FITKGraphObjectShapeOCC* occObj = dynamic_cast<Exchange::FITKGraphObjectShapeOCC*>(obj);

        // 添加可视化对象。
        //@{
        if (vtkW && vtkObj)
        {
            vtkW->addObject(vtkObj->getRenderLayer(), vtkObj, fitView);
        }

        if (occW && occObj)
        {
            occW->addObject(occObj, fitView);
        }
        //@}
    }
}  // namespace GUIOper