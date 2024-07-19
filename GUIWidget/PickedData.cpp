#include "PickedData.h"

// Qt
#include <QVector>

// VTK
#include <vtkPlanes.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSet.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkMapper.h>
#include <vtkSelectionNode.h>
#include <vtkSelection.h>
#include <vtkExtractSelection.h>
#include <vtkIdTypeArray.h>

// APP
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKKeyMouseStates.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Graph
#include "FITK_Interface/FITKVTKAlgorithm/FITKGraphActor.h"
#include "FITK_Interface/FITKVTKAlgorithm/FITKGraphActor2D.h"
#include "FITK_Component/FITKOCC2VTKGraphAdaptor/FITKOCC2VTKGraphObjectShape.h"

// Filter ( Algorithm )
#include "FITK_Interface/FITKVTKAlgorithm/FITKShellFeatureEdges.h"
#include "FITK_Interface/FITKVTKAlgorithm/FITKSurfaceFilter.h"
#include "FITK_Interface/FITKVTKAlgorithm/FITKDataSetReader.h"

// GUI
#include "GUIPickInfo.h"

namespace GraphData
{
    PickedData::PickedData(GUI::GUIPickInfoStru pickedInfo, vtkActor* pickedActor, int pickedIndex, double* pickedWorldPos, bool isPreview) :
        m_pickedInfo(pickedInfo), m_pickedActor(pickedActor), m_pickedIndex(pickedIndex), m_isPreview(isPreview)
    {
        // 初始化鼠标操作方式。（点击）
        m_mouseOper = PickedMouseType::PickedMouseClick;

        if (pickedWorldPos)
        {
            m_pickedWorldPos[0] = pickedWorldPos[0];
            m_pickedWorldPos[1] = pickedWorldPos[1];
            m_pickedWorldPos[2] = pickedWorldPos[2];
        }

        // 初始化。
        init();
    }

    PickedData::PickedData(GUI::GUIPickInfoStru pickedInfo, vtkActor* pickedActor, vtkPlanes* cutPlane) :
        m_pickedInfo(pickedInfo), m_pickedActor(pickedActor), m_cutPlane(cutPlane)
    {
        // 初始化鼠标操作方式。（框选）
        m_mouseOper = PickedMouseType::PickedMouseRubber;

        // 初始化。
        init();
    }

    PickedData::PickedData(GUI::GUIPickInfoStru pickedInfo, vtkActor2D* pickedActor, bool isPreview) :
        m_pickedInfo(pickedInfo), m_pickedActor2D(pickedActor), m_isPreview(isPreview)
    {
        // 初始化鼠标操作方式。（点击）
        m_mouseOper = PickedMouseType::PickedMouseClick;

        // 初始化。
        init2D();
    }

    PickedData::PickedData()
    {

    }

    PickedData::~PickedData()
    {
        // 析构前恢复高亮（参考点、坐标轴）状态。
        clearHighlight();
    }

    PickedData* PickedData::getCopy()
    {
        if (m_needToCal || !m_isValid)
        {
            return nullptr;
        }

        PickedData* data = new PickedData;
        data->Type = Type;
        data->DataObjId = DataObjId;

        data->Ids = Ids;
        data->GraphObject = GraphObject;

        data->m_pickedActor = m_pickedActor;
        data->m_pickedActor2D = m_pickedActor2D;
        data->m_pickedIndex = m_pickedIndex;
        data->m_pickedInfo = m_pickedInfo;

        return data;
    }

    bool PickedData::isSameAs(PickedData* data)
    {
        if (!data)
        {
            return false;
        }

        if (data->Type == Type &&
            data->getPickedActor() == m_pickedActor &&
            // data->CaseId == CaseId &&
            data->DataObjId == DataObjId)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void PickedData::sort()
    {
        std::sort(Ids.begin(), Ids.end());
    }

    void PickedData::add(PickedData* data)
    {
        if (!data)
        {
            return;
        }

        // 首先进行排序，获取ID最大值。（最后一位ID）
        sort();
        data->sort();

        // 获取最大ID开数组存取存在标识。
        if (Ids.count() && data->Ids.count())
        {
            int nIdsMax = qMax(Ids.last(), data->Ids.last());
            QVector<int> idFlag(nIdsMax);
            idFlag.fill(-1);
            for (const int & id : Ids)
            {
                idFlag[id - 1] = 1;
            }

            // 合并单元或节点ID。
            for (const int & id : data->Ids)
            {
                if (idFlag[id - 1] == -1)
                {
                    Ids.push_back(id);
                    idFlag[id - 1] = 1;
                }
            }
        }

        // 合并后析构。
        delete data;
    }

    void PickedData::subtract(PickedData* data)
    {
        if (!data)
        {
            return;
        }

        // 首先进行排序，获取ID最大值。（最后一位ID）
        sort();
        data->sort();

        // 获取最大ID开数组存取存在标识。
        if (Ids.count() && data->Ids.count())
        {
            int nIdsMax = qMax(Ids.last(), data->Ids.last());
            QVector<int> idFlags(nIdsMax);
            idFlags.fill(-1);
            for (const int & id : Ids)
            {
                idFlags[id - 1] = 1;
            }

            // 移除单元或节点ID。
            for (const int & id : data->Ids)
            {
                if (idFlags[id - 1] == 1)
                {
                    idFlags[id - 1] = -1;
                }
            }

            Ids.clear();
            for (int i = 0; i < idFlags.count(); i++)
            {
                int& idFlag = idFlags[i];
                if (idFlag == 1)
                {
                    Ids.push_back(i + 1);
                }
            }
        }

        // 合并后析构。
        delete data;
    }

    void PickedData::init()
    {
        // 尝试转换为自定义Actor。
        FITKGraphActor* fActor = FITKGraphActor::SafeDownCast(m_pickedActor);
        if (!fActor)
        {
            return;
        }

        // 框选到特征边则跳出。
        if (fActor->getActorType() == ActorType::EdgeActor && m_mouseOper == PickedMouseType::PickedMouseRubber)
        {
            return;
        }

        // 获取演员存储的可视化对象。
        Exchange::FITKOCC2VTKGraphObjectShape* obj = fActor->getGraphObjectAs<Exchange::FITKOCC2VTKGraphObjectShape>();
        if (!obj)
        {
            return;
        }

        // 保存可视化对象。
        GraphObject = obj;
        DataObjId = obj->getDataId();

        ShapeInfo sInfo = obj->getShapeInfo();

        // 后续需增加枚举转换方法。
        Type = PickedDataType(sInfo.Type);
        DataObjId = sInfo.DataObjId;

        if (Type == OtherPick)
        {
            return;
        }

        m_needToCal = true;
        m_isValid = true;
    }

    void PickedData::init2D()
    {
        // 尝试转换为自定义Actor。
        FITKGraphActor2D* fActor2D = FITKGraphActor2D::SafeDownCast(m_pickedActor2D);
        if (!fActor2D)
        {
            return;
        }

        // 获取演员存储的可视化对象。
        Exchange::FITKOCC2VTKGraphObjectShape* obj = fActor2D->getGraphObjectAs<Exchange::FITKOCC2VTKGraphObjectShape>();
        if (!obj)
        {
            return;
        }

        // 保存可视化对象。
        GraphObject = obj;
        DataObjId = obj->getDataId();

        ShapeInfo sInfo = obj->getShapeInfo();

        // 后续需增加枚举转换方法。
        Type = PickedDataType(sInfo.Type);
        DataObjId = sInfo.DataObjId;

        if (Type == OtherPick)
        {
            return;
        }

        m_needToCal = false;
        m_isValid = true;
    }

    void PickedData::highlight()
    {
        if (!GraphObject)
        {
            return;
        }

        // 如果是预选并且没有在高亮则预选。
        if (m_isPreview)
        {
            if (!GraphObject->isHighlighting())
            {
                GraphObject->preHighlight();
            }
        }
        else
        {
            GraphObject->highlight();
        }
    }

    void PickedData::clearHighlight()
    {
        // 取消高亮。
        if (!GraphObject)
        {
            return;
        }

        // 如果是预选高亮参考点需要额外判断是否为高亮状态。
        if (m_isPreview)
        {
            // 如果为高亮状态则还原为高亮。
            if (GraphObject->isHighlighting())
            {
                GraphObject->highlight();
            }
            // 否则取消预选高亮。
            else
            {
                GraphObject->disHighlight();
            }
        }
        // 高亮则直接取消高亮。
        else
        {
            GraphObject->disHighlight();
        }
    }

    void PickedData::calculateFinsish()
    {
        m_needToCal = false;

        m_mouseOper = PickedMouseType::PickedNoneType;
        m_pickedIndex = -1;
        m_cutPlane = nullptr;

        m_pickedWorldPos[0] = 0;
        m_pickedWorldPos[1] = 0;
        m_pickedWorldPos[2] = 0;
    }

    bool PickedData::needToCalculate()
    {
        return m_needToCal;
    }

    bool PickedData::isValid()
    {
        return m_isValid;
    }

    bool PickedData::isEmpty()
    {
        // 如果数据对象ID为空则一定为空拾取数据。
        if (DataObjId < 0)
        {
            return true;
        }

        bool isEmpty = Ids.isEmpty();
        return isEmpty;
    }

    bool PickedData::contains(vtkActor* actor, int index)
    {
        if (!actor || index < 0 || !GraphObject)
        {
            return false;
        }

        // 首先判断是否为同一演员，参考点直接返回。
        if (actor != m_pickedActor)
        {
            return false;
        } 

        int id = -1;

        switch (m_pickedInfo._pickObjType)
        {
        case GUI::GUIPickInfo::PickObjType::POBJVert:
            id = GraphObject->getOCCIdByVTKCellId(index, TopAbs_ShapeEnum::TopAbs_VERTEX);
            break;
        case GUI::GUIPickInfo::PickObjType::POBJEdge:
            id = GraphObject->getOCCIdByVTKCellId(index, TopAbs_ShapeEnum::TopAbs_EDGE);
            break;
        case GUI::GUIPickInfo::PickObjType::POBJFace:
            id = GraphObject->getOCCIdByVTKCellId(index, TopAbs_ShapeEnum::TopAbs_FACE);
            break;
        case GUI::GUIPickInfo::PickObjType::POBJSolid:
            id = GraphObject->getOCCIdByVTKCellId(index, TopAbs_ShapeEnum::TopAbs_SOLID);
            break;
        {
            return false;
        }
        }

        if (id == -1)
        {
            return false;
        }

        return Ids.contains(id);
    }

    bool PickedData::contains(vtkActor2D* actor)
    {
        if (!actor || !GraphObject)
        {
            return false;
        }

        // 新版本
        //@{
        return GraphObject->contains(actor);
        //@}
    }

    GUI::GUIPickInfoStru PickedData::getPickedInfo()
    {
        return m_pickedInfo;
    }

    vtkActor* PickedData::getPickedActor()
    {
        return m_pickedActor;
    }

    int PickedData::getPickedIndex()
    {
        return m_pickedIndex;
    }

    PickedMouseType PickedData::getPickedMouseType()
    {
        return m_mouseOper;
    }

    vtkPlanes* PickedData::getCutPlane()
    {
        return m_cutPlane;
    }

    void PickedData::getDataSet(vtkUnstructuredGrid* ugrid)
    {
        // 未计算过的数据直接跳出。
        if (m_needToCal || !ugrid || !GraphObject)
        {
            return;
        }

        vtkDataSet* dataSet{ nullptr };
        TopAbs_ShapeEnum shapeEnum;

        // 根据拾取模型数据类型获取数据集。
        switch (Type)
        {
        case PickedDataType::ModelVertPick:
        {
            dataSet = GraphObject->getMesh(ShapeType::ModelVertex);
            shapeEnum = TopAbs_ShapeEnum::TopAbs_VERTEX;
            break;
        }
        case PickedDataType::ModelEdgePick:
        {
            dataSet = GraphObject->getMesh(ShapeType::ModelEdge);
            shapeEnum = TopAbs_ShapeEnum::TopAbs_EDGE;
            break;
        }
        case PickedDataType::ModelFacePick:
        {
            dataSet = GraphObject->getMesh(ShapeType::ModelFace);
            shapeEnum = TopAbs_ShapeEnum::TopAbs_FACE;
            break;
        }
        case PickedDataType::ModelSolidPick:
        {
            dataSet = GraphObject->getMesh(ShapeType::ModelSolid);
            shapeEnum = TopAbs_ShapeEnum::TopAbs_SOLID;
            break;
        }
        default:
            return;
        }

        if (!dataSet)
        {
            return;
        }

        int nCell = dataSet->GetNumberOfCells();

        // 创建ID数组，加速合并数据。
        vtkSmartPointer<vtkIntArray> idArray = vtkSmartPointer<vtkIntArray>::New();
        idArray->SetNumberOfComponents(1);
        idArray->SetNumberOfValues(nCell);
        idArray->FillComponent(0, 0);

        // 根据OCC形状ID获取所有VTK数据。
        for (const int & id : Ids)
        {
            const QVector<int> subCellIds = this->GraphObject->getVTKCellIdsByOCCId(id, shapeEnum);
            for (const int & cId : subCellIds)
            {
                idArray->SetValue(cId, 1);
            }
        }

        // 通过ID获取实际VTK索引。
        vtkIdTypeArray* selectIdArray = vtkIdTypeArray::New();
        vtkSelectionNode* selectNode = vtkSelectionNode::New();
        vtkExtractSelection* extractSelection = vtkExtractSelection::New();

        for (int i = 0; i < nCell; i++)
        {
            if (idArray->GetValue(i) == 1)
            {
                selectIdArray->InsertNextValue(i);
            }
        }

        // 从原始网格数据提取。
        selectNode->SetFieldType(vtkSelectionNode::SelectionField::CELL);
        extractSelection->SetInputData(dataSet);

        // 提取VTK数据。
        //@{
        vtkSelection* section = vtkSelection::New();
        selectNode->SetContentType(vtkSelectionNode::INDICES);
        section->AddNode(selectNode);
        extractSelection->SetInputData(1, section);
        selectNode->SetSelectionList(selectIdArray);
        extractSelection->Update();
        //@}

        // 数据拷贝。
        ugrid->DeepCopy(extractSelection->GetOutput());

        // 析构。
        //@{
        selectNode->Delete();
        section->Delete();
        selectIdArray->Delete();
        extractSelection->Delete();
        //@}
    }

    void PickedData::getPickedWorldPosition(double* pos)
    {
        if (!pos)
        {
            return;
        }

        for (int i = 0; i < 3; i++)
        {
            pos[i] = m_pickedWorldPos[i];
        }
    }
}