#include "OperGraphEvent3D.h"

// App
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

// Graph
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObjectShape.h"

// Render VTK
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphRender.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphObjectVTK.h"

// Render OCC
//#include "FITK_Component/FITKRenderWindowOCC/FITKGraph3DWindowOCC.h"

// GUI
#include "GUIFrame/MainWindow.h"
#include "GUIFrame/RenderWidget.h"
#include "FITK_Component/FITKWidget/FITKMdiArea.h"

namespace GUIOper
{
    Comp::FITKGraph3DWindowVTK* OperGraphEvent3D::getGraphWidget()
    {
        // 获取主窗口。
        GUI::MainWindow* mainWindow = FITKAPP->getGlobalData()->getMainWindowT<GUI::MainWindow>();
        if (!mainWindow)
        {
            return nullptr;
        }

        // 获取可视化区。
        GUI::RenderWidget* renderWidget = mainWindow->getRenderWidget();
        if (!renderWidget)
        {
            return nullptr;
        }

        Comp::FITKMdiArea* mdiArea = renderWidget->getMdiArea();
        if (!mdiArea)
        {
            return nullptr;
        }

        // 获取当前窗口，尝试转换为三维窗口。
        QWidget* w = mdiArea->getCurrentWidget();
        Comp::FITKGraph3DWindowVTK* graphWidget = dynamic_cast<Comp::FITKGraph3DWindowVTK*>(w);

        return graphWidget;
    }

    void OperGraphEvent3D::addGraphObjectToWidget(Exchange::FITKOCC2VTKGraphObjectShape* obj, Comp::FITKGraph3DWindowVTK* graphWidget, bool fitView)
    {
        if (!obj || !graphWidget)
        {
            return;
        }

        // 添加可视化对象。
        //@{
        obj->removeFromGraphWidget();
        graphWidget->addObject(obj->getRenderLayer(), obj, true);
        //}

        // 添加附加可视化对象。
        //@{
        QList<Exchange::FITKOCC2VTKGraphObjectShape*> addinObjs = obj->getAddinGraphObjs();
        for (Exchange::FITKOCC2VTKGraphObjectShape* addinObj : addinObjs)
        {
            if (!addinObj)
            {
                continue;
            }

            addinObj->removeFromGraphWidget();
            graphWidget->addObject(addinObj->getRenderLayer(), addinObj, true);
        }
        //}

        // 刷新窗口。
        //@{
        if (fitView)
        {
            graphWidget->fitView();
        }
        else
        {
            graphWidget->reRender();
        }
        //@}
    }
}  // namespace GUIOper