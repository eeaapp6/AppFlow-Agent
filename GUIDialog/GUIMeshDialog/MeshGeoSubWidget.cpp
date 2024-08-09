#include "MeshGeoSubWidget.h"
#include "ui_MeshGeoSubWidget.h"

namespace GUI
{
    MeshGeoSubWidget::MeshGeoSubWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUIWidgetBase(parent)
    {
        _ui = new Ui::MeshGeoSubWidget();
        _ui->setupUi(this);
    }

    MeshGeoSubWidget::~MeshGeoSubWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void MeshGeoSubWidget::setName(const QString & name)
    {
        _ui->groupBox_Geo->setTitle(name);
    }

    void MeshGeoSubWidget::setObjID(int id)
    {
        _objID = id;
    }

    int MeshGeoSubWidget::getObjID()
    {
        return _objID;
    }

    void MeshGeoSubWidget::init()
    {

    }
}

