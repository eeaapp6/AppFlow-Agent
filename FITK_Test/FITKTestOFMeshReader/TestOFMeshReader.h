#ifndef __FITKTESTMESH_H__
#define __FITKTESTMESH_H__

#include <QObject>

namespace Interface {
    class FITKOpenFOAMMeshReader;
}
namespace FITKTest
{
    class TestOFMeshReader : public QObject
    {
        Q_OBJECT
    public:
        TestOFMeshReader() = default;
        ~TestOFMeshReader() = default;

    private slots:
        void initTestCase();
        void cleanupTestCase();

        void testReader();

    private:
        Interface::FITKOpenFOAMMeshReader* _reader{};
    };
}

#endif
