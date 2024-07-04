#include "GlobalDataFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"


Core::FITKAbstractDataObject *GlobalDataFactory::createMeshData()
{
    // 不创建网格数据
    return nullptr;
}

Core::FITKAbstractDataObject* GlobalDataFactory::createGeoData()
{
    // 创建几何数据
    return new Interface::FITKOFGeometryData;
}

Core::FITKAbstractDataObject *GlobalDataFactory::createPhysicsData()
{
    // 创建abaqus数据
    return nullptr;
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
