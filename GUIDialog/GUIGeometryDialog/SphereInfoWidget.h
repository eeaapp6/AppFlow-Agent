#ifndef _SphereInfoWidget_H
#define _SphereInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Ui {
    class SphereInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelSphere;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIGeometryDialogAPI SphereInfoWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        //创建构造函数
        SphereInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        //编辑构造函数
        SphereInfoWidget(Interface::FITKAbsGeoModelSphere* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~SphereInfoWidget();

        void init();

        void setCenterPoint(double* point);

        void setFaceGroupValue(int rowIndex, QList<int> facesId);

        Interface::FITKAbsGeoCommand* getCurrentGeoCommand();
    private slots:
        ;
        void on_pushButton_CenterPoint_clicked();

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
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void closeEvent(QCloseEvent *event);

    private:
        bool checkValue();
        void setDataToWidget();
        void getDataFormWidget();
        //更新表格标题
        void updateTableTitle();
        //初始化表格
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
    private:
        bool _isCreate = false;
        Interface::FITKAbsGeoModelSphere* _obj = nullptr;
        Ui::SphereInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
