#ifndef _DataSwitchToWidget_H
#define _DataSwitchToWidget_H

#include "GUICalculateDialogAPI.h"

class QWidget;
class QTableWidget;

namespace Interface {
    class FITKFlowDataBase;
}

namespace GUI
{
    GUICalculateDialogAPI QWidget* DataSwitchToWidget(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
    GUICalculateDialogAPI QTableWidget* DataGroupSwitchToWidget(QWidget* parent = nullptr);
}

#endif
