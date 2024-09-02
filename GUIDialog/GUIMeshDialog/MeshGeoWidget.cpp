#include "MeshGeoWidget.h"
#include "ui_MeshGeoWidget.h"
#include "MeshGeoSubWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Component/FITKWidget/FITKTabWidget.h"

#include <QVBoxLayout>
#include <QSpacerItem>

namespace GUI
{
    MeshGeoWidget::MeshGeoWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _ui = new Ui::MeshGeoWidget();
        _ui->setupUi(this);
        init();
    }

    MeshGeoWidget::~MeshGeoWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void MeshGeoWidget::init()
    {
        Comp::FITKTabWidget* tabWidget = new Comp::FITKTabWidget(Comp::FITKTabWidgetType::FITKTab_Auto, this);
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;
        for (int i = 0; i < geometryData->getDataCount(); i++) {
            auto geoData = geometryData->getDataByIndex(i);
            if (geoData == nullptr)continue;
            MeshGeoSubWidget* subWidget = new MeshGeoSubWidget(geoData, _oper, tabWidget);
            tabWidget->addTab(subWidget, geoData->getDataObjectName());
        }
        _ui->verticalLayout->addWidget(tabWidget);
    }
}

