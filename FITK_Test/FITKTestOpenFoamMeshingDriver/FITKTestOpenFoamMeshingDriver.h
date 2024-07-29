#ifndef __FITKTESTMESH_H__
#define __FITKTESTMESH_H__

#include <QObject>

namespace Interface {
    class FITKOpenFOAMMeshReader;
}
namespace FITKTest
{
    class FITKTestOpenFoamMeshingDriver : public QObject
    {
        Q_OBJECT
    public:
        explicit FITKTestOpenFoamMeshingDriver();
        ~FITKTestOpenFoamMeshingDriver();
        void testRun();

    private slots:
        void initTestCase();
        void blockMeshTestCase();
        void snappyHexMeshTestCase();

    private:
    };
}

#endif
