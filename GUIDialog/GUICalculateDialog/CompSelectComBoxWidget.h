/**********************************************************************
 * @file   CompSelectComBoxWidget.h
 * @brief  comBox选择组件界面
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-08-26
 *********************************************************************/
#ifndef _CompSelectComBoxWidget_H
#define _CompSelectComBoxWidget_H

#include <QWidget>

namespace Ui {
    class CompSelectComBoxWidget;
}

namespace Interface {
    class FITKAbstractParameter;
}

namespace GUI {

    /**
     * @brief  获取不同子数据函数指针声明
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-08-26
     */
    typedef Interface::FITKAbstractParameter* (*getSubDataFormText)(const QString&, int);

    /**
     * @brief  comBox选择组件界面
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-08-26
     */
    class CompSelectComBoxWidget :public QWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] type combox名称
         * @param[i] parent 父对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        CompSelectComBoxWidget(QString type, QWidget* parent = nullptr);
        /**
         * @brief    析构函数
         * @return   void
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        ~CompSelectComBoxWidget();
        /**
         * @brief    初始化
         * @return   void
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void init();
        /**
         * @brief    更新
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void update();
        /**
         * @brief    设置函数指针
         * @param[i] function  函数指针 
         * @param[i] index  索引值（solution需要）
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void setFunction(getSubDataFormText function, int index = -1);
        /**
         * @brief    设置选项
         * @param[i] Options  选项
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void setOptions(QStringList Options);
        /**
         * @brief    设置当前索引
         * @param[i] index   索引
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void setCurrentText(const QString index);
        /**
         * @brief    设置子界面数据
         * @param[i] data 子界面数据
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void setSubWidgetData(Interface::FITKAbstractParameter* data);
    private slots:
        ;
        /**
         * @brief    隐藏/显示子界面按钮点击槽函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void on_pushButton_clicked();
        /**
         * @brief    类型选择槽函数
         * @param[i] index 当前索引
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void on_comboBox_currentIndexChanged(int index);
    private:
        /**
         * @brief  ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        Ui::CompSelectComBoxWidget* _ui = nullptr;
        /**
         * @brief  类型
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        QString _type = "";
        /**
         * @brief  函数指针
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        getSubDataFormText _myFunction = nullptr;
        /**
         * @brief  solution切换时需要索引值（目前solution专用）
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        int _index = -1;
    };
}

#endif
