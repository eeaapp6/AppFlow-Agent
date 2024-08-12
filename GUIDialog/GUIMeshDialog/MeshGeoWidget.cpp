#include "MeshGeoWidget.h"
#include "MeshGeoSubWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"

#include <QVBoxLayout>
#include <QSpacerItem>

namespace GUI
{
    MeshGeoWidget::MeshGeoWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _subWidgetLayout = new QVBoxLayout(this);
        init();
    }

    MeshGeoWidget::~MeshGeoWidget()
    {

    }

    void MeshGeoWidget::init()
    {
        updateSubWidget();
    }

    void MeshGeoWidget::updateSubWidget()
    {
        Interface::FITKGeoCommandList* geoList = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geoList == nullptr)return;

        for (int i = 0; i < geoList->getDataCount(); i++) {
            auto geoCom = geoList->getDataByIndex(i);
            if (geoCom == nullptr)continue;

            MeshGeoSubWidget* subWidget = new MeshGeoSubWidget(geoCom->getDataObjectID(), _oper, this);
            subWidget->setName(geoCom->getDataObjectName());
            _subWidgetLayout->addWidget(subWidget);
        }
        QSpacerItem* spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        _subWidgetLayout->addItem(spacerItem);
    }
}

