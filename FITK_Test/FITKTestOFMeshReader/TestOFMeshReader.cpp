#include "TestOFMeshReader.h"
#include "FITK_OFMesh/FITKOpenFOAMMeshReader.h"
#include <QTest>
#include <QThread>

namespace FITKTest
{
    void TestOFMeshReader::testReader()
    {
        _reader = new Interface::FITKOpenFOAMMeshReader;

        _reader->setFileName("C:\\Users\\chan\\Desktop\\motorBike\\constant\\polyMesh\\");
        _reader->run();
    }

    void TestOFMeshReader::initTestCase()
    {

    }

    void TestOFMeshReader::cleanupTestCase()
    {
        delete _reader;
    }

}
