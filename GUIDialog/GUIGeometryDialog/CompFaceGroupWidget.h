#ifndef _CompFaceGroupWidget_H
#define _CompFaceGroupWidget_H

#include "GUIWidget/GUIWidgetBase.h"

class QTableWidgetItem;

namespace Ui {
    class CompFaceGroupWidget;
}

namespace Interface {
    class FITKAbsGeoCommand;
    class FITKAbstractGeoModel;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class CompFaceGroupWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        //编辑构造函数
        CompFaceGroupWidget(QWidget* paraent, Interface::FITKAbsGeoCommand* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~CompFaceGroupWidget();

        void init();
        //设置面组
        void setFaceGroupValue(int rowIndex, QList<int> facesId);
        //获取当前数据对象
        Interface::FITKAbsGeoCommand* getCurrentGeoCommand();

        void setDataToWidget();
        /**
         * @brief 清除所有高亮
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void clearGraphHight();
    private slots:
        ;
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
    private:
        bool checkValue();
        //更新表格标题
        //void updateTableTitle();
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
         * @brief 清除Table
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void clearTableWidget();
    private:
        Interface::FITKAbsGeoCommand* _obj = nullptr;
        Ui::CompFaceGroupWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
