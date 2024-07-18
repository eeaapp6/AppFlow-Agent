#include "CudeInfoWidget.h"
#include "ui_CudeInfoWidget.h"

#include "GUIFrame/MainWindow.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI {
    
    CudeInfoWidget::CudeInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true)
    {
        init();
    }

    CudeInfoWidget::CudeInfoWidget(Interface::FITKAbsGeoModelBox * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true), _obj(obj)
    {
        init();
    }

    CudeInfoWidget::~CudeInfoWidget()
    {
        if (_ui)delete _ui;
    }
    
    void CudeInfoWidget::init()
    {
        _ui = new Ui::CudeInfoWidget();
        _ui->setupUi(this);
    }
}

