#include "WidgetOCCEvent.h"
#include "PickedData.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFHexMeshBaseMeshBox.h"
#include "FITK_Component/FITKGeoCompOCC/FITKAbstractOCCModel.h"
#include "FITK_Component/FITKGeoCompOCC/FITKOCCModelSimpleShape.h"

#include <TopoDs_Vertex.hxx>
#include <TopoDs.hxx>
#include <BRep_Tool.hxx>

namespace GUI
{
    WidgetOCCEvent::WidgetOCCEvent()
    {

    }

    WidgetOCCEvent::~WidgetOCCEvent()
    {

    }

    void WidgetOCCEvent::getPoint(GraphData::PickedData* data, double* point, bool isOk)
    {
        QList<int> ids = data->getPickedIds();
        int DataObjId = data->getPickedDataObjId();
        GraphData::PickedDataType type = data->getPickedDataType();
        if (ids.size() == 0) {
            isOk = false;
            return;
        }

        //数据仓库中获取数据
        Interface::FITKAbstractModel* model = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKAbstractModel>(DataObjId);
        TopoDS_Shape shape;

        OCC::FITKAbstractOCCModel* OCCModel = dynamic_cast<OCC::FITKAbstractOCCModel*>(model);
        if (OCCModel == nullptr) {
            isOk = false;
            return;
        }
        shape = OCCModel->getShape(Interface::FITKModelEnum::FMSPoint, ids[0]);

        if (shape.IsNull()) {
            isOk = false;
            return;
        }

        TopoDS_Vertex vertex = TopoDS::Vertex(shape);
        gp_Pnt pt = BRep_Tool::Pnt(vertex);

        point[0] = pt.X();
        point[1] = pt.Y();
        point[2] = pt.Z();
    }
}
