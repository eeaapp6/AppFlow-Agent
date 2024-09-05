#include "OperatingConditionWidget.h"
#include "ui_OperatingConditionWidget.h"
#include "CompCalLineWidget.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFOperatingConditions.h"

namespace GUI
{
    OperatingConditionWidget::OperatingConditionWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::OperatingConditionWidget();
        _ui->setupUi(this);

        init();
    }

    OperatingConditionWidget::~OperatingConditionWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void OperatingConditionWidget::init()
    {
        if (_physicsData == nullptr)return;
        _operCondition = _physicsData->getOperatingConditions();
        updateWidget();
    }

    void OperatingConditionWidget::updateWidget()
    {
        if (_operCondition == nullptr)return;
        _ui->groupBox_GraAcc->hide();
        _ui->groupBox_RefPre->hide();
        //清除全部子界面
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_GraAcc->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }
        while ((item = _ui->verticalLayout_RefPre->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }

       Interface::FITKAbstractParameter* graAccData = _operCondition->getGravitationalAcceleration();
       if (graAccData) {
           if (graAccData->getParameterCount() > 0)_ui->groupBox_GraAcc->show();
           for (auto d : graAccData->getParameter()) {
               if (d == nullptr)continue;
               QWidget* widget = new CompCalLineWidget(d, this);
               _ui->verticalLayout_GraAcc->addWidget(widget);
           }
       }

       Interface::FITKAbstractParameter* refPreData = _operCondition->getReferencePressure();
       if (refPreData) {
           if (refPreData->getParameterCount() > 0)_ui->groupBox_RefPre->show();
           for (auto d : refPreData->getParameter()) {
               if (d == nullptr)continue;
               QWidget* widget = nullptr;
               auto type = d->getDataType();
               switch (type){
               case Interface::FlowDataType::FLowDataRadioGroup:
               case Interface::FlowDataType::FLowDataBoolGroup: {
                   widget = CompCalLineWidget::DataSwitchToWidget(d, this); break;
                   break;
               }
               default:widget = new CompCalLineWidget(d, this); break;
               }
               _ui->verticalLayout_RefPre->addWidget(widget);
           }
       }
    }
}

