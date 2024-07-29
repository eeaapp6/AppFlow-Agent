#include "FITKTestOpenFoamMeshingDriver.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAbstractCommandRunner.h"
#include "FITK_Kernel/FITKAppFramework/FITKAbstractProgramDriver.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramTaskManager.h"
#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"
#include <QStringList>
//#include "FITK_Component/FITKOpenFoamMeshingDriver/FITKOFBlockMeshDriver.h"
//#include "FITK_Component/FITKOpenFoamMeshingDriver/FITKOFSnappyHexMeshDriver.h"

#include <QTest>
#include <QThread>
#include <QDebug>
namespace FITKTest
{
    FITKTestOpenFoamMeshingDriver::FITKTestOpenFoamMeshingDriver()
    {
    }
    FITKTestOpenFoamMeshingDriver::~FITKTestOpenFoamMeshingDriver()
    {
    }

    void FITKTestOpenFoamMeshingDriver::testRun()
    {
        qDebug() << "create openFoamTestCase";
        initTestCase();
        blockMeshTestCase();
        snappyHexMeshTestCase();
    }

    void FITKTestOpenFoamMeshingDriver::initTestCase()
    {
        AppFrame::FITKLinuxCommandRunner cpDir;
        qint64 pid = -1;
        AppFrame::RunStatus runStatus;
        qDebug() << cpDir.executeCommand("rm -rf $HOME/openFoamTestCase", pid, runStatus) << pid;

        qDebug() << cpDir.executeCommand("mkdir $HOME/openFoamTestCase", pid, runStatus) << pid;
        qDebug() << cpDir.executeCommand("cp -r $FOAM_APP/../tutorials/incompressibleFluid/motorBike $HOME/openFoamTestCase/", pid, runStatus) << pid;
        //copy geometry
        qDebug() << cpDir.executeCommand("cp -r $FOAM_TUTORIALS/resources/geometry/motorBike.obj.gz $HOME/openFoamTestCase/motorBike/motorBike/constant/geometry/", pid, runStatus) << pid;
    }

    void FITKTestOpenFoamMeshingDriver::blockMeshTestCase()
    {
        auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
        auto proGramManager = app->getProgramTaskManager();
        AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
        QStringList args;
        args << "-case" << "$HOME/openFoamTestCase/motorBike/motorBike";
        info->setArgs(args);
        auto progam = proGramManager->createProgram(1, "FITKOFBlockMeshDriver", info);
        if (!progam) return;
        
        connect(progam, &AppFrame::FITKAbstractProgramerDriver::sig_Finish, this, [=] 
        {
            qDebug() << "Paraview opening please wait ...";
            AppFrame::FITKLinuxCommandRunner opParaview;
            qint64 pid = -1;
            AppFrame::RunStatus runStatus;
            opParaview.executeCommand("paraFoam -case $HOME/openFoamTestCase/motorBike/motorBike", pid, runStatus);
        
        });
        progam->start();
    }

    void FITKTestOpenFoamMeshingDriver::snappyHexMeshTestCase()
    {
        auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
        auto proGramManager = app->getProgramTaskManager();
        AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
        QStringList args;
        args << "-overwrite" << "-case" << "$HOME/openFoamTestCase/motorBike/motorBike";
        info->setArgs(args);
        auto progam = proGramManager->createProgram(1, "FITKOFSnappyHexMeshDriver", info);
        if (!progam) return;

        connect(progam, &AppFrame::FITKAbstractProgramerDriver::sig_Finish, this, [=]
        {
            qDebug() << "Paraview opening please wait ...";
            AppFrame::FITKLinuxCommandRunner opParaview;
            qint64 pid = -1;
            AppFrame::RunStatus runStatus;
            opParaview.executeCommand("paraFoam -case $HOME/openFoamTestCase/motorBike/motorBike", pid, runStatus);

        });
        progam->start();
    }

}
