#include "TurbulenceWidget.h"
#include "ui_TurbulenceWidget.h"
#include "compCalLineWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFCasePhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTurbulenceData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowSolverProcessFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKSolverSimpleTemplate.h"

#include <QButtonGroup>
#include <QScrollArea>

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
        //界面默认设置
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

        //更具数据对象初始化界面
        setDataToWidget();
        connect(_radioGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotRadioButtonClicked()));
    }

    void TurbulenceWidget::updateSubWidget()
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

        //清除子参数
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_ModelSub->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }
        while ((item = _ui->verticalLayout_DeltaSub->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }

        //重新添加子参数
        if (_turData->getCurrentTurbulenceModelData()) {
            for (int i = 0; i < _turData->getCurrentTurbulenceModelData()->getParameterCount(); i++) {
                auto dataBase = _turData->getCurrentTurbulenceModelData()->getParameterAt(i);
                if (dataBase == nullptr)continue;
                _ui->verticalLayout_ModelSub->addWidget(new compCalLineWidget(dataBase, this));
            }
        }
        if (_turData->getDeltaData(_ui->comboBox_Delta->currentText())) {
            auto deltaData = _turData->getDeltaData(_ui->comboBox_Delta->currentText());
            for (int i = 0; i < deltaData->getParameterCount(); i++) {
                auto dataBase = deltaData->getParameterAt(i);
                if (dataBase == nullptr)continue;
                _ui->verticalLayout_DeltaSub->addWidget(new compCalLineWidget(dataBase, this));
            }
        }
    }

    void TurbulenceWidget::setDataToWidget()
    {
        if (_turData == nullptr)return;

        auto Modelingtype = _turData->getTurbulenceModelingType();
        QStringList ModelTypes = _factor->getSolverTemplate()->getTurbulenceModelTypeList(Modelingtype);
        _ui->comboBox_Model->clear();
        _ui->comboBox_Model->addItems(ModelTypes);
        if (_turData->getCurrentTurbulenceModelData()) {
            _ui->comboBox_Model->setCurrentText(_turData->getCurrentModelType());

            QStringList DeltaTypes = _factor->getSolverTemplate()->getTurbulenceDeltaTypeList(_turData->getCurrentModelType());
            _ui->comboBox_Delta->clear();
            _ui->comboBox_Delta->addItems(DeltaTypes);
            QString deltaType = _turData->getCurrentDeltaType();
            if (!deltaType.isEmpty()) {
                _ui->comboBox_Delta->setCurrentText(deltaType);
            }
            else {
                _ui->comboBox_Delta->setCurrentIndex(0);
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
        else if (_radioGroup->checkedButton() == _ui->radioButton_RANS) {
            _ui->widget_sub->show();
            _ui->checkBox_Enable->setChecked(false);
            _turData->setEnableTurbulenceEquations(false);
            type = Interface::FITKOFSolverTurbulenceEnum::RAS;

            QStringList types = _factor->getSolverTemplate()->getTurbulenceModelTypeList(type);
            _ui->comboBox_Model->clear();
            _ui->comboBox_Model->addItems(types);
            if (_turData->getCurrentTurbulenceModelData()) {
                _ui->comboBox_Model->setCurrentText(_turData->getCurrentModelType());
            }
            else {
                _ui->comboBox_Model->setCurrentIndex(0);
            }

            QString type = _ui->comboBox_Model->currentText();
            _factor->setTurbulenceModel(type);
        }
        else if (_radioGroup->checkedButton() == _ui->radioButton_LES) {
            _ui->widget_sub->show();
            _ui->checkBox_Enable->setChecked(false);
            _turData->setEnableTurbulenceEquations(false);
            type = Interface::FITKOFSolverTurbulenceEnum::LES;

            QStringList types = _factor->getSolverTemplate()->getTurbulenceModelTypeList(type);
            _ui->comboBox_Model->clear();
            _ui->comboBox_Model->addItems(types);
            if (_turData->getCurrentTurbulenceModelData()) {
                _ui->comboBox_Model->setCurrentText(_turData->getCurrentModelType());
            }
            else {
                _ui->comboBox_Model->setCurrentIndex(0);
            }

            QString type = _ui->comboBox_Model->currentText();
            _factor->setTurbulenceModel(type);
        }

        _turData->setTurbulenceModelingType(type);

        updateSubWidget();
    }

    void TurbulenceWidget::on_comboBox_Model_activated(int index)
    {
        QString type = _ui->comboBox_Model->currentText();
        _factor->setTurbulenceModel(type);

        QStringList deltaTyps = _factor->getSolverTemplate()->getTurbulenceDeltaTypeList(type);
        if (deltaTyps.isEmpty()) {
            _ui->widget_Delta->hide();
        }
        else
        {
            _ui->widget_Delta->show();
            _ui->comboBox_Delta->addItems(deltaTyps);
        }

        updateSubWidget();
    }

    void TurbulenceWidget::on_pushButton_ModelUnfold_clicked()
    {
        if (_ui->pushButton_ModelUnfold->isChecked()) {
            _ui->widget_ModelSub->show();
        }
        else {
            _ui->widget_ModelSub->hide();
        }

        updateSubWidget();
    }

    void TurbulenceWidget::on_comboBox_Delta_activated(int index)
    {
        updateSubWidget();
    }

    void TurbulenceWidget::on_pushButton_DeltaUnfold_clicked()
    {
        if (_ui->pushButton_DeltaUnfold->isChecked()) {
            _ui->widget_DeltaSub->show();
        }
        else {
            _ui->widget_DeltaSub->hide();
        }

        updateSubWidget();
    }
}
