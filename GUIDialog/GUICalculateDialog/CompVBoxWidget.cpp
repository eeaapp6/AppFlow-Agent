#include "CompVBoxWidget.h"
#include "ui_CompVBoxWidget.h"
#include "CompCalLineWidget.h"

#include <QRadioButton>

namespace GUI
{
    CompVBoxWidget::CompVBoxWidget(QList<QWidget*> widgetList, QWidget * parent) :
        QWidget(parent), _subWidgets(widgetList)
    {
        _ui = new Ui::CompVBoxWidget();
        _ui->setupUi(this);

        for (int i = 0; i < widgetList.size(); i++) {
            auto w = widgetList[i];
            if (w == nullptr)continue;
            _ui->verticalLayout->addWidget(w);
        }
    }

    CompVBoxWidget::~CompVBoxWidget()
    {
        if (_ui)delete _ui;
    }

    void CompVBoxWidget::setSubWidgetEnable(bool enable, bool isFirst)
    {
        for (int i = 0; i < _subWidgets.size(); i++) {
            auto w = _subWidgets[i];
            if (w == nullptr)continue;
            if (i == 0 && isFirst == true) {
                continue;
            }
            w->setEnabled(enable);
        }
    }
}

