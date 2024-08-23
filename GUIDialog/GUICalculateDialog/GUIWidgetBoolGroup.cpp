#include "GUIWidgetBoolGroup.h"
#include "ui_GUIWidgetBoolGroup.h"
#include "CompCalLineWidget.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBoolGroup.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"

namespace GUI {

    GUIWidgetBoolGroup::GUIWidgetBoolGroup(Interface::FITKFlowDataBoolGroup * dataBase, QWidget * parent) :
        QWidget(parent), _dataBase(dataBase)
    {
        _ui = new Ui::GUIWidgetBoolGroup();
        _ui->setupUi(this);
        init();
    }

    GUIWidgetBoolGroup::~GUIWidgetBoolGroup()
    {
        if (_ui)delete _ui;
    }

    void GUIWidgetBoolGroup::init()
    {
        if (_dataBase == nullptr)return;

        _ui->widget_Sub->hide();

        updateWidget();
    }

    void GUIWidgetBoolGroup::on_checkBox_clicked()
    {
        if (_dataBase == nullptr)return;
        _dataBase->setValue(_ui->checkBox->isChecked());
        updateWidget();
    }

    void GUIWidgetBoolGroup::updateWidget()
    {
        if (!_dataBase)return;

        QString name = _dataBase->getDataObjectName();
        _ui->label_Name->setText(name);

        //控制子界面是否显示
        if (_dataBase->getValue()) {
            _ui->widget_Sub->show();
            _ui->checkBox->setChecked(true);
        }
        else {
            _ui->widget_Sub->hide();
            _ui->checkBox->setChecked(false);
        }

        //清除子参数
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_Sub->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }
        //子界面添加数据
        if (!_dataBase->getValueGroup())return;
        for (auto v : _dataBase->getValueGroup()->getParameter()) {
            if (!v)continue;
            QWidget* w = new CompCalLineWidget(v, this);
            _ui->verticalLayout_Sub->addWidget(w);
        }
    }
}