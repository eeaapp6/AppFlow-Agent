#ifndef _CylinderInfoWidget_H
#define _CylinderInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

class QTableWidgetItem;

namespace Ui {
    class CylinderInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelCylinder;
    class FITKAbstractGeoModel;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIGeometryDialogAPI CylinderInfoWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        //创建构造函数
        CylinderInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        //编辑构造函数
        CylinderInfoWidget(Interface::FITKAbsGeoModelCylinder* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~CylinderInfoWidget();

        void init();

        void setOriginPoint(double* point);

        void setFaceGroupValue(int rowIndex, QList<int> facesId);

        Interface::FITKAbsGeoCommand* getCurrentGeoCommand();
    private slots:
        ;
        void on_pushButton_OriginPoint_clicked();

        void on_pushButton_Cancel_clicked();

        void on_pushButton_CreateOrEdit_clicked();

        void on_pushButton_Clear_clicked();

        void on_pushButton_Add_clicked();
        /**
         * @brief 表格点击事件
         * @param[i]  item           表格对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotCellTableClicked(int row, int column);
        /**
         * @brief 面组名称修改事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotEditNameStart();
        void slotEditNameFinish();
        /**
         * @brief 面组界面ok点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotFaceWidgetOkClicked();
        /**
         * @brief 面组界面cancel点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotFaceWidgetCancelClicked();
        /**
         * @brief 面组界面delete点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotFaceWidgetDeleteClicked();
        /**
         * @brief 关闭事件重写
         * @param[i]  event          事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void closeEvent(QCloseEvent *event);
    private:
        bool checkValue();
        void setDataToWidget();
        void getDataFormWidget();
        void initTableWidget();

        void setAllFaceGroupSelect(bool type);
        /**
         * @brief 刷新面组界面记录的位置
         * （为解决点击界面控件时，QTableWidget未触发不知道当前界面所在的位置问题）
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void updateFaceWidgetCurrentPos();
        /**
         * @brief 清除所有高亮
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void clearGraphHight();
        /**
         * @brief 清除Table
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void clearTableWidget();
        /**
         * @brief 更新几何划分网格尺寸数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void updateMeshGeoMeshSize();
        /**
         * @brief 更新几何划分网格尺寸数据中记录的面组id
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void updateMeshGeoMeshSizeID();
    private:
        bool _isCreate = false;
        Interface::FITKAbsGeoModelCylinder* _obj = nullptr;
        Interface::FITKAbstractGeoModel* _geoModel = nullptr;
        Ui::CylinderInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
