#ifndef _TabWidget_H
#define _TabWidget_H

#include "GUIFrameAPI.h"
#include <QTableWidget>

namespace GUI
{
    class GUIFRAMEAPI TabWidget :public QTabWidget
    {
        Q_OBJECT;
    public:
        TabWidget(QWidget* parent);
        ~TabWidget();
    };
}

#endif
