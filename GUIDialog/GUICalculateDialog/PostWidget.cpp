#include "PostWidget.h"
#include "ui_PostWidget.h"
#include "RunProcess.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"

#include <QFile>
#include <QTextStream>

namespace GUI
{
    PostWidget::PostWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _ui = new Ui::PostWidget();
        _ui->setupUi(this);
    }

    PostWidget::~PostWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void GUI::PostWidget::on_pushButton_ParaView_clicked()
    {
        //工作路径获取
        QString workDir = "";
        if (FITKAPP->getAppSettings()) {
            workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (workDir.isEmpty()) workDir = QApplication::applicationDirPath() + "/../WorkDir";
        QString caseDir = workDir + "/case";

        QString foamFile = creatStartParaViewFile(caseDir);
        QString sh = QString("paraview --case %1").arg(foamFile);

        //启动进程
        RunProcess* currentPro = new RunProcess();
        //进程结束信号处理
        connect(currentPro, &RunProcess::sigFinish, [=]() {
            if (currentPro) {
                delete currentPro;
            }
        });
        currentPro->start(sh);
    }

    QString PostWidget::creatStartParaViewFile(QString caseDir)
    {
        Core::CreateDir(caseDir);

        //创建case.foam文件用于paraView启动，查看后处理结果
        QString foamFile = caseDir + "/case.foam";

        QFile file(foamFile);
        // 打开文件进行写操作
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return "";
        // 创建一个 QTextStream 对象来写入文本
        QTextStream out(&file);
        // 关闭文件
        file.close();
        return foamFile;
    }
}
