#include "TestOFMeshReader.h"
#include "FITK_OFMesh/FITKOpenFOAMMeshReader.h"
#include <QTest>
#include <QThread>

namespace FITKTest
{
    TestOFMeshReader::TestOFMeshReader(QString path)
        : _path(path)
    {

    }
    TestOFMeshReader::~TestOFMeshReader() {
        delete _reader;
        _reader = nullptr;
    }

    void TestOFMeshReader::testRun()
    {
        if (_path.isEmpty()) return;
        _reader = new Interface::FITKOpenFOAMMeshReader;
        _reader->setFileName(_path);
        _reader->run();
    }

    void TestOFMeshReader::testReader()
    {
        _reader = new Interface::FITKOpenFOAMMeshReader;
        _reader->setFileName("C:/Users/chan/Desktop/cavity/constant/polyMesh/");
        _reader->run();
    }

    void TestOFMeshReader::initTestCase()
    {

    }

    void TestOFMeshReader::cleanupTestCase()
    {

    }

}
