#ifndef _PostWidget_H
#define _PostWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"
#include <QProcess>

namespace Ui {
    class PostWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUICalculateDialogAPI PostWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        PostWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~PostWidget();

    private slots:
        ;
        /**
         * @brief    ParaView打开按钮
         * @return   void
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        void on_pushButton_ParaView_clicked();
    private:
        /**
         * @brief    创建启paraView可以启动openFOAM的结果文件
         * @param[i] caseDir   算例路径
         * @return   QString   
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        QString creatStartParaViewFile(QString caseDir);
    private:
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        Ui::PostWidget* _ui = nullptr;
    };
}
#endif
