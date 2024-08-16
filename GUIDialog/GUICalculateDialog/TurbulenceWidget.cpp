#include "TurbulenceWidget.h"
#include "ui_TurbulenceWidget.h"
#include "compCalLineWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolverData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTurbulenceData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"

#include <QButtonGroup>

namespace GUI
{
    TurbulenceWidget::TurbulenceWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::TurbulenceWidget();
        _ui->setupUi(this);

        if (_solverData) {
            auto data = _solverData->getSolverSettingData(Interface::FITKOFPostProcessEnum::FITKOFSolverRequiresSettingType::Turbulence);
            _turData = dynamic_cast<Interface::FITKOFTurbulenceData*>(data);
        }

        init();
    }

    TurbulenceWidget::~TurbulenceWidget()
    {
        if (_radioGroup) delete _radioGroup;
        if (_ui)  delete _ui;
    }

    void TurbulenceWidget::init()
    {
        _ui->checkBox_Enable->setChecked(true);

        _radioGroup = new QButtonGroup();
        _radioGroup->addButton(_ui->radioButton_Laminar);
        _radioGroup->addButton(_ui->radioButton_RANS);
        _radioGroup->addButton(_ui->radioButton_LES);
        _ui->radioButton_Laminar->setChecked(true);

        _ui->widget_sub->hide();

        //默认隐藏湍流模型参数
        _ui->pushButton_ModelUnfold->setCheckable(true);
        _ui->pushButton_DeltaUnfold->setCheckable(true);
        _ui->widget_ModelSub->hide();
        _ui->widget_DeltaSub->hide();

        updateWidget();

        connect(_radioGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotRadioButtonClicked()));
    }

    void TurbulenceWidget::updateWidget()
    {
        if (_turData == nullptr)return;

        _ui->checkBox_Enable->setChecked(_turData->isEnableTurbulenceEquations());

        auto type = _turData->getTurbulenceModelingType();
        switch (type) {
        case Interface::FITKOFSolverTurbulenceEnum::Laminar:_ui->radioButton_Laminar->setChecked(true); break;
        case Interface::FITKOFSolverTurbulenceEnum::RAS: {
            _ui->radioButton_RANS->setChecked(true);
            _ui->widget_sub->show();
            break;
        }
        case Interface::FITKOFSolverTurbulenceEnum::LES:{
            _ui->radioButton_LES->setChecked(true);
            _ui->widget_sub->show();
            break;
        }
        }
    }

    void TurbulenceWidget::on_checkBox_Enable_clicked()
    {
        if (_turData == nullptr)return;
        _turData->setEnableTurbulenceEquations(_ui->checkBox_Enable->isChecked());
    }

    void TurbulenceWidget::slotRadioButtonClicked()
    {
        if (_turData == nullptr)return;

        Interface::FITKOFSolverTurbulenceEnum::FITKOFTurbulenceModelType type;
        if (_radioGroup->checkedButton() == _ui->radioButton_Laminar) {
            _ui->widget_sub->hide();
            type = Interface::FITKOFSolverTurbulenceEnum::Laminar;
        }
        else if(_radioGroup->checkedButton() == _ui->radioButton_RANS){
            _ui->widget_sub->show();
            _ui->checkBox_Enable->setChecked(false);
            _turData->setEnableTurbulenceEquations(false);
            type = Interface::FITKOFSolverTurbulenceEnum::RAS;
        }
        else if (_radioGroup->checkedButton() == _ui->radioButton_LES) {
            _ui->widget_sub->show();
            _ui->checkBox_Enable->setChecked(false);
            _turData->setEnableTurbulenceEquations(false);
            type = Interface::FITKOFSolverTurbulenceEnum::LES;
        }

        _turData->setTurbulenceModelingType(type);
    }

    void TurbulenceWidget::on_comboBox_Model_activated(int index)
    {
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_ModelSub->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }
    }

    void TurbulenceWidget::on_pushButton_ModelUnfold_clicked()
    {
        if (_ui->pushButton_ModelUnfold->isChecked()) {
            _ui->widget_ModelSub->show();
        }
        else {
            _ui->widget_ModelSub->hide();
        }
    }

    void TurbulenceWidget::on_comboBox_Delta_activated(int index)
    {
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_DeltaSub->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
    }

    void TurbulenceWidget::on_pushButton_DeltaUnfold_clicked()
    {
        if (_ui->pushButton_DeltaUnfold->isChecked()) {
            _ui->widget_DeltaSub->show();
        }
        else {
            _ui->widget_DeltaSub->hide();
        }
    }
}
