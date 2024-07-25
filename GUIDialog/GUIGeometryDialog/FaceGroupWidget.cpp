#include "FaceGroupWidget.h"
#include "ui_FaceGroupWidget.h"

#include <QApplication>
#include <QStyle>

namespace GUI
{
    GUI::FaceGroupWidget::FaceGroupWidget(QWidget * parent):
        QWidget(parent)
    {
        _ui = new Ui::FaceGroupWidget();
        _ui->setupUi(this);

        init();

        connect(_ui->pushButton_OK, SIGNAL(clicked()), this, SIGNAL(sigOkClicked()));
        connect(_ui->pushButton_Cancel, SIGNAL(clicked()), this, SIGNAL(sigCancelClicked()));
        connect(_ui->pushButton_Delete, SIGNAL(clicked()), this, SIGNAL(sigDeleteClicked()));
    }

    GUI::FaceGroupWidget::~FaceGroupWidget()
    {
        if (_ui)delete _ui;
        _data.clear();
    }

    void FaceGroupWidget::init()
    {
        _currentPos.first = -1;
        _currentPos.second = -1;

        _ui->pushButton_OK->setStyleSheet("background: transparent;");
        _ui->pushButton_OK->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
        _ui->pushButton_OK->hide();

        _ui->pushButton_Cancel->setStyleSheet("background: transparent;");
        _ui->pushButton_Cancel->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogBack));
        _ui->pushButton_Cancel->hide();

        _ui->pushButton_Delete->setStyleSheet("background: transparent;");
        _ui->pushButton_Delete->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    }

    void FaceGroupWidget::setName(QString name)
    {
        _ui->label->setText(name);
    }

    QString FaceGroupWidget::getName()
    {
        return _ui->label->text();
    }

    void FaceGroupWidget::setData(int pos, QVariant value)
    {
        _data.insert(pos, value);
    }

    QVariant FaceGroupWidget::data(int pos)
    {
        return _data.value(pos);
    }
    void FaceGroupWidget::setSelect(bool type)
    {
        if (type) {
            _ui->pushButton_OK->show();
            _ui->pushButton_Cancel->show();
        }
        else
        {
            _ui->pushButton_OK->hide();
            _ui->pushButton_Cancel->hide();
        }
    }
    void FaceGroupWidget::setCurrentPos(int row, int clo)
    {
        _currentPos.first = row;
        _currentPos.second = clo;
    }
    QPair<int, int> FaceGroupWidget::getCurrentPos()
    {
        return _currentPos;
    }
}

