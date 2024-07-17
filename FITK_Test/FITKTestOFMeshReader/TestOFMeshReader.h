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
        explicit TestOFMeshReader(QString path = QString());
        ~TestOFMeshReader();

        void testRun();

    private slots:
        void initTestCase();
        void cleanupTestCase();
        void testReader();

    private:
        Interface::FITKOpenFOAMMeshReader* _reader{};
        QString _path{};
    };
}

#endif
