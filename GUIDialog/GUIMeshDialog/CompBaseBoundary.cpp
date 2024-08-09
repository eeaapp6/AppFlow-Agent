#include "CompBaseBoundary.h"

#include <QPainter>
#include <QHeaderView>

namespace GUI
{
    CompBaseBoundaryLabel::CompBaseBoundaryLabel(QWidget* parent) :
        QLabel(parent)
    {
        //设置鼠标跟踪
        setMouseTracking(true);
    }

    CompBaseBoundaryLabel::~CompBaseBoundaryLabel()
    {

    }

    void CompBaseBoundaryLabel::setPos(int pos)
    {
        _pos = pos;
    }

    int CompBaseBoundaryLabel::getPos()
    {
        return _pos;
    }

    void CompBaseBoundaryLabel::mouseMoveEvent(QMouseEvent * event)
    {
        QLabel::mouseMoveEvent(event);
        Q_UNUSED(event);
        emit sigMouseMove();
    }

    CompBaseBoundaryComboBox::CompBaseBoundaryComboBox(QWidget* parent) :
        QComboBox(parent)
    {
        //设置鼠标跟踪
        setMouseTracking(true);
    }

    CompBaseBoundaryComboBox::~CompBaseBoundaryComboBox()
    {

    }

    void CompBaseBoundaryComboBox::setPos(int pos)
    {
        _pos = pos;
    }

    int CompBaseBoundaryComboBox::getPos()
    {
        return _pos;
    }

    void CompBaseBoundaryComboBox::mouseMoveEvent(QMouseEvent * event)
    {
        QComboBox::mouseMoveEvent(event);

        Q_UNUSED(event); 
        emit sigMouseMove();
    }
}

