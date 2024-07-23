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

    double* WidgetOCCEvent::getPoint(GraphData::PickedData* data, bool isOk)
    {
        double value[3] = { 0,0,0 };

        QList<int> ids = data->Ids;
        int DataObjId = data->DataObjId;
        GraphData::PickedDataType type = data->Type;
        if (ids.size() == 0) {
            isOk = false;
            return value;
        }

        //数据仓库中获取数据
        Interface::FITKAbstractModel* model = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKAbstractModel>(DataObjId);
        TopoDS_Shape shape;

        OCC::FITKAbstractOCCModel* OCCModel = dynamic_cast<OCC::FITKAbstractOCCModel*>(model);
        if (OCCModel == nullptr) {
            isOk = false;
            return value;
        }
        shape = OCCModel->getShape(Interface::FITKModelEnum::FMSPoint, ids[0]);

        if (shape.IsNull()) {
            isOk = false;
            return value;
        }

        TopoDS_Vertex vertex = TopoDS::Vertex(shape);
        gp_Pnt pt = BRep_Tool::Pnt(vertex);

        value[0] = pt.X();
        value[1] = pt.Y();
        value[2] = pt.Z();
        return value;
    }
}
