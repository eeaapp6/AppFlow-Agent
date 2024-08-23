#include "compVBoxWidget.h"
#include "ui_compVBoxWidget.h"
#include "compCalLineWidget.h"

#include <QRadioButton>

namespace GUI
{
    compVBoxWidget::compVBoxWidget(QList<QWidget*> widgetList, QWidget * parent) :
        QWidget(parent), _subWidgets(widgetList)
    {
        _ui = new Ui::compVBoxWidget();
        _ui->setupUi(this);

        for (int i = 0; i < widgetList.size(); i++) {
            auto w = widgetList[i];
            if (w == nullptr)continue;
            _ui->verticalLayout->addWidget(w);
        }
    }

    compVBoxWidget::~compVBoxWidget()
    {
        if (_ui)delete _ui;
    }

    void compVBoxWidget::setSubWidgetEnable(bool enable, bool isFirst)
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

