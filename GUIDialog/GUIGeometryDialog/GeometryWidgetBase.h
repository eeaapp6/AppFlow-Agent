#ifndef GeometryWidgetBase_H_
#define GeometryWidgetBase_H_

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace GUI
{
    class MainWindow;

    class GUIGeometryDialogAPI GeometryWidgetBase : public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        GeometryWidgetBase(QWidget* parent);
        virtual ~GeometryWidgetBase();
    protected:
        void showEvent(QShowEvent *event) override;
        void closeEvent(QCloseEvent* event) override;
        void transparency(bool geo = true, bool mesh = true);
    private:
        MainWindow* _mainWin = nullptr;
    };
}

#endif
