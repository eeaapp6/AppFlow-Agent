#ifndef _MeshBaseTypeWidgetBase_H
#define _MeshBaseTypeWidgetBase_H

#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Interface {
    class FITKAbstractRegionMeshSize;
}

namespace GUI 
{
	class MeshBaseWidget;
	class MainWindow;

    class MeshBaseTypeWidgetBase :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
		MeshBaseTypeWidgetBase(QWidget* parent);
        virtual ~MeshBaseTypeWidgetBase();

        virtual bool checkValue() = 0;

        virtual bool setDataToWidget(Interface::FITKAbstractRegionMeshSize* obj) = 0;

        virtual bool getDataFromWidget(Interface::FITKAbstractRegionMeshSize* obj) = 0;

        virtual void updateGeometryGraph() = 0;

    protected slots:
        ;
        void slotMouseMove();
    protected:
        virtual void clearBoundaryBackgroudColor() = 0;
	protected:
		MainWindow* _mainWin = nullptr;
		MeshBaseWidget* _meshBaseWidget = nullptr;

        //临时数据对象
        Interface::FITKAbstractRegionMeshSize* _graphObj = nullptr;
    };
}

#endif
