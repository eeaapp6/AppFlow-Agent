#include "MainWindowGenerator.h"
// #include "FITK_Kernal/FITKAppFramework/FITKAppFramework.h"
// #include "FITK_Kernal/FITKAppFramework/FITKCommandLineHandler.h"

#include <QMainWindow>

MainWindowGenerator::MainWindowGenerator()
{
//    this->setStyle("://Structural.qss");
//    this->showMaximize(false);
}

QWidget * MainWindowGenerator::genMainWindow()
{
    return new QMainWindow;
}
