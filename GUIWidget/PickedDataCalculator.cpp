#include "PickedDataCalculator.h"

// VTK
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkPlanes.h>

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
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObject3D.h"

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
            case GUI::GUIPickInfo::PickMethod::PMSingle:
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

        // 排序。
        m_pickedData->sortIds();
    }

    void PickedDataCalculator::individually()
    {
        Exchange::FITKOCC2VTKGraphObject3D* gobj = m_pickedData->getPickedGraphObejct();
        int index = m_pickedData->getPickedIndex();
        if (!gobj || index < 0)
        {
            return;
        }

        int id = -1;

        // 根据拾取数据类型进行不同数据获取。
        switch (m_pickedData->getPickedDataType())
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

        m_pickedData->getPickedIds().push_back(id);
    } 

    void PickedDataCalculator::byAreaPick()
    {
        Exchange::FITKOCC2VTKGraphObject3D* gobj = m_pickedData->getPickedGraphObejct();
        vtkPlanes* planes = m_pickedData->getCutPlane();
        vtkActor* actor = m_pickedData->getPickedActor();
        if (!gobj || !planes || !actor)
        {
            return;
        }

        // 获取拾取数据集。
        vtkDataSet* dataSet = actor->GetMapper()->GetInputAsDataSet();
        if (!dataSet)
        {
            return;
        }

        vtkSmartPointer<FITKExtractGeometry> extractor = vtkSmartPointer<FITKExtractGeometry>::New();
        extractor->SetImplicitFunction(planes);
        extractor->SetInputData(dataSet);
        extractor->Update();

        // 通过单元索引获取单元ID。
        const QList<int> cellsIndice = extractor->getSelectOriginalCells();
        if (cellsIndice.isEmpty())
        {
            return;
        }

        // 拾取状态数组。（加速数据判断包含）
        QVector<int> flags;
        int len = 0;        
        ShapeAbsEnum sType;

        // 根据拾取数据类型进行不同数据获取。
        switch (m_pickedData->getPickedDataType())
        {
        case PickedDataType::ModelVertPick:
            len = gobj->getNumberOf(ShapeType::ModelVertex);
            sType = ShapeAbsEnum::STA_VERTEX;
            break;
        case PickedDataType::ModelEdgePick:
            len = gobj->getNumberOf(ShapeType::ModelEdge);
            sType = ShapeAbsEnum::STA_EDGE;
            break;
        case PickedDataType::ModelFacePick:
            len = gobj->getNumberOf(ShapeType::ModelFace);
            sType = ShapeAbsEnum::STA_FACE;
            break;
        case PickedDataType::ModelSolidPick:
            len = gobj->getNumberOf(ShapeType::ModelSolid);
            sType = ShapeAbsEnum::STA_SOLID;
            break;
        default:
            return;
        }

        if (len == 0)
        {
            return;
        }

        // OCC数据Id从1开始，需额外开一位数字。
        flags.resize(len + 1);
        flags.fill(0);

        // 预处理拾取单元数据。（加速判断拾取子Id包含关系）
        int nCells = dataSet->GetNumberOfCells();
        QVector<int> cellPickedFlags;
        cellPickedFlags.resize(nCells);
        cellPickedFlags.fill(0);

        // 获取OCC数据Id。
        for (const int & index : cellsIndice)
        {
            int id = gobj->getOCCIdByVTKCellId(index, sType);
            flags[id] = 1;
            cellPickedFlags[index] = 1;
        }

        // 保存拾取数据。
        for (int i = 1; i <= len; i++)
        {
            if (flags[i])
            {
                // 检测当前OCC数据是否完全被选中。
                QVector<int> subIds = gobj->getVTKCellIdsByOCCId(i, sType);

                bool isFullPicked = true;
                for (const int & id : subIds)
                {
                    isFullPicked &= (cellPickedFlags[id] == 1);
                }

                // 完全选中则视为被框选。
                if (isFullPicked)
                {
                    m_pickedData->getPickedIds().push_back(i);
                }
            }
        }

        flags.clear();
    }
}