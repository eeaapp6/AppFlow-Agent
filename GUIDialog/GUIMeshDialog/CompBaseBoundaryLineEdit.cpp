#include "CompBaseBoundaryLineEdit.h"

namespace GUI {
    CompBaseBoundaryLineEdit::CompBaseBoundaryLineEdit(QWidget* parent):
        QLineEdit(parent)
    {
        //设置鼠标跟踪
        setMouseTracking(true);
    }

    CompBaseBoundaryLineEdit::~CompBaseBoundaryLineEdit()
    {

    }

    void CompBaseBoundaryLineEdit::setPos(int pos)
    {
        _pos = pos;
    }

    int CompBaseBoundaryLineEdit::getPos()
    {
        return _pos;
    }

    void CompBaseBoundaryLineEdit::mouseMoveEvent(QMouseEvent* event)
    {
        emit sigMouseMove();
        QLineEdit::mouseMoveEvent(event);
    }
}
