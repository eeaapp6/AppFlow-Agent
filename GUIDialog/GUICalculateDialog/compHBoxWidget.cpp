#include "compHBoxWidget.h"
#include "ui_compHBoxWidget.h"
#include "compCalLineWidget.h"

#include <QRadioButton>

namespace GUI
{
    compHBoxWidget::compHBoxWidget(QList<QWidget*> widgetList, QWidget * parent) :
        QWidget(parent), _subWidgets(widgetList)
    {
        _ui = new Ui::compHBoxWidget();
        _ui->setupUi(this);

        for (int i = 0; i < widgetList.size(); i++) {
            auto w = widgetList[i];
            if (w == nullptr)continue;
            _ui->horizontalLayout->addWidget(w);
        }
    }

    compHBoxWidget::~compHBoxWidget()
    {
        if (_ui)delete _ui;
    }

    void compHBoxWidget::setSubWidgetEnable(bool enable, bool isFirst)
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

