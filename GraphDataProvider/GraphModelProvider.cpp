#include "GraphModelProvider.h"

// Render VTK
#include "FITK_GeneralComponent/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_GeneralComponent/FITKRenderWindowVTK/FITKGraphRender.h"
#include "FITK_GeneralComponent/FITKRenderWindowVTK/FITKGraphObjectVTK.h"

// Render OCC
#include "FITK_GeneralComponent/FITKRenderWindowOCC/FITKGraphObjectOCC.h"

// Graph
#include "FITK_GeneralComponent/FITKOCCGraphAdaptor/FITKGraphObjectShapeOCC.h"
#include "FITK_GeneralComponent/FITKOCCGraphAdaptor/FITKGraphObjectShapeVTK.h"

// Graph widget
#include "FITK_Kernal/FITKCore/FITKAbstractGraphWidget.h"

namespace GraphData
{
    GraphModelProvider::GraphModelProvider()
    {

    }

    GraphModelProvider::~GraphModelProvider()
    {
        // 析构三维可视化对象。
    }

    QString GraphModelProvider::getClassName()
    {
        return "GraphModelProvider";
    }

    QList<Core::FITKAbstractGraphObject*> GraphModelProvider::getCurrentGraphObjs()
    {
        // 当前所有模型可视化对象数据。
        QList<Core::FITKAbstractGraphObject*> objs;

        // objs << .values();

        return objs;
    }
}   // namespace GraphData