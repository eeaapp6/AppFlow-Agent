#ifndef _RunWidget_H
#define _RunWidget_H

#include <QWidget>
#include "GUICalculateDialogAPI.h"

class QAbstractButton;

namespace Ui {
    class RunWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    enum class RunCPUType {
        Serial,
        Parallel,
    };

    class GUICalculateDialogAPI RunWidget : public QWidget
    {
        Q_OBJECT;
    public:
        RunWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~RunWidget();

        void init();
    private slots:
        ;
        void slotCPUChange(QAbstractButton* button);
        void slotProcessFinish();
        void on_spinBox_NumOfPro_valueChanged(int arg1);

        void on_pushButton_Stop_clicked();
        void on_pushButton_Run_clicked();


    private:
        void initCPU();
        void updateCPU();
        /**
         * @brief    清理算例文件
         * @param[i] casePath  算例路径 
         * @return   bool      是否成功
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-29
         */
        bool clearCasePath(QString casePath);
        /**
         * @brief    写出算例
         * @param[i] casePath  算例路径 
         * @return   bool      是否成功
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-29
         */
        bool writeCase(QString casePath);
        /**
         * @brief    创建启动脚本
         * @param[i] workDir   工作路径
         * @param[i] caseDir   算例路劲
         * @return   QString   脚本路径
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        QString creatStartSh(QString workDir, QString caseDir);

        void setRunType(bool isRun);
    private:
        Ui::RunWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
