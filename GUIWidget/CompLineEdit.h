#ifndef CompLineEdit_H
#define CompLineEdit_H

#include "GUIWidgetAPI.h"
#include <QLineEdit>

namespace GUI
{
    class GUIWIDGETAPI CompLineEdit : public QLineEdit
    {
        Q_OBJECT;
    public:
        CompLineEdit(QWidget* widget);
        ~CompLineEdit();

    signals:
        ;
        void sigEditStart();
        void sigEditFinish();
    private:
        //鼠标双击事件重写
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        //失焦事件重写
        void focusOutEvent(QFocusEvent* event) override;
        //回车事件
        void keyPressEvent(QKeyEvent* event) override;
    private:
        bool _isEdit = false;
    };
}

#endif
