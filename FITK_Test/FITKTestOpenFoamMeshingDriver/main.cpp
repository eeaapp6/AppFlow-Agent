#include <QTest>
#include <QApplication>  
#include <iostream>
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITKTestOpenFoamMeshingDriver.h"

int main(int argc, char* argv[])
{
    AppFrame::FITKApplication app(argc, argv);
    auto test = new FITKTest::FITKTestOpenFoamMeshingDriver();
    test->testRun();
    return app.exec();
}
