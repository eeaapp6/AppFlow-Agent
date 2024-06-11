#include "MainWindowGenerator.h"
#include "GUIFrame/MainWindow.h"

MainWindowGenerator::MainWindowGenerator()
{
	//    this->setStyle("://Structural.qss");
	//    this->showMaximize(false);
}

QWidget * MainWindowGenerator::genMainWindow()
{
	return new GUI::MainWindow;
}
