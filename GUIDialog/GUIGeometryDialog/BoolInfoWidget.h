#ifndef _BoolInfoWidget_H
#define _BoolInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "GeometryWidgetBase.h"

class QTableWidgetItem;

namespace Ui {
    class BoolInfoWidget;
}

namespace Interface {
    class FITKAbsGeoOperBool;
    class FITKAbstractGeoModel;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIGeometryDialogAPI BoolInfoWidget :public GeometryWidgetBase
    {
        Q_OBJECT;
    public:
        //编辑构造函数
        BoolInfoWidget(Interface::FITKAbsGeoOperBool* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~BoolInfoWidget();

        void init();
        //设置面组
        void setFaceGroupValue(int rowIndex, QList<int> facesId);
        //获取当前数据对象
        Interface::FITKAbsGeoCommand* getCurrentGeoCommand();
    protected:
        /**
         * @brief 关闭事件重写
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void closeEvent(QCloseEvent *event) override;
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
        void setDataToWidget();
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
        Interface::FITKAbsGeoOperBool* _obj = nullptr;
        Ui::BoolInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
