#include "GlobalDataFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"


Core::FITKAbstractDataObject *GlobalDataFactory::createMeshData()
{
    return new Interface::FITKUnstructuredFluidMeshVTK;
}

Core::FITKAbstractDataObject* GlobalDataFactory::createGeoData()
{
    // 创建几何数据
    return new Interface::FITKOFGeometryData;
}

Core::FITKAbstractDataObject *GlobalDataFactory::createPhysicsData()
{
    // 创建算例求解的物理场数据
    return new Interface::FITKOFPhysicsData;
}

Core::FITKAbstractDataObject *GlobalDataFactory::createPostData()
{
    // 创建后处理数据
    return nullptr;
}

QHash<int, Core::FITKAbstractDataObject *> GlobalDataFactory::createOtherData()
{
    // 不创建其他数据
    return QHash<int, Core::FITKAbstractDataObject *>();
}
