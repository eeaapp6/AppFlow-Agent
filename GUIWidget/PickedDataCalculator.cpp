#include "PickedDataCalculator.h"

// VTK
#include <vtkCellData.h>
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPlanes.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkPolygon.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkOBBTree.h>
#include <vtkPointData.h>
#include <vtkLine.h>

// APP
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKKeyMouseStates.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Filter ( Algorithm )
#include "FITK_Interface/FITKVTKAlgorithm/FITKShellFeatureEdges.h"
#include "FITK_Interface/FITKVTKAlgorithm/FITKSurfaceFilter.h"
#include "FITK_Interface/FITKVTKAlgorithm/FITKExtractGeometry.h"

// Graph
#include "FITK_Interface/FITKVTKAlgorithm/FITKGraphActor.h"
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKCommons.h"
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObjectShape.h"

// Pick
#include "PickedData.h"

namespace GraphData
{
    PickedDataCalculator::PickedDataCalculator(GraphData::PickedData* pickedData) :
        m_pickedData(pickedData)
    {

    }

    void PickedDataCalculator::calculate()
    {
        if (!m_pickedData)
        {
            return;
        }

        // 点选拾取方式
        //@{
        if (m_pickedData->getPickedMouseType() == PickedMouseType::PickedMouseClick)
        {
            // 获取拾取时的方式。
            GUI::GUIPickInfoStru pickInfo = m_pickedData->getPickedInfo();
            switch (pickInfo._pickMethod)
            {
            case GUI::GUIPickInfo::PickMethod::PMIndividually:
            {
                individually();
                break;
            }
            default:
                return;
            }
        }
        //@}
        // 框选拾取方式
        //@{
        else if (m_pickedData->getPickedMouseType() == PickedMouseType::PickedMouseRubber)
        {
            byAreaPick();
        }
        //@}

        // 不管计算完成都将是否需要计算标识置为false。
        m_pickedData->calculateFinsish();
    }

    void PickedDataCalculator::individually()
    {
        Exchange::FITKOCC2VTKGraphObjectShape* gobj = m_pickedData->GraphObject;
        int index = m_pickedData->getPickedIndex();
        if (!gobj || index < 0)
        {
            return;
        }

        int id = -1;

        // 根据拾取数据类型进行不同数据获取。
        switch (m_pickedData->Type)
        {
        case PickedDataType::ModelVertPick:
            // 查找点。
            id = gobj->getOCCIdByVTKCellId(index, ShapeAbsEnum::STA_VERTEX);
            break;
        case PickedDataType::ModelEdgePick:
            // 查找线。
            id = gobj->getOCCIdByVTKCellId(index, ShapeAbsEnum::STA_EDGE);
            break;
        case PickedDataType::ModelFacePick:
            // 查找面。
            id = gobj->getOCCIdByVTKCellId(index, ShapeAbsEnum::STA_FACE);
            break;
        case PickedDataType::ModelSolidPick:
            // 查找体。
            id = gobj->getOCCIdByVTKCellId(index, ShapeAbsEnum::STA_SOLID);
            break;
        default:
            return;
        }

        if (id == -1)
        {
            return;
        }

        m_pickedData->Ids.push_back(id);
    } 

    void PickedDataCalculator::byAreaPick()
    {
        // 根据拾取数据类型进行不同数据获取。
        switch (m_pickedData->Type)
        {
            // 部件与装配实例相同。
        case PickedDataType::ModelVertPick:
        case PickedDataType::ModelEdgePick:
        case PickedDataType::ModelFacePick:
        case PickedDataType::ModelSolidPick:
        default:
            return;
        }
    }
}