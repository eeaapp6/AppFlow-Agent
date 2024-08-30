#include "PostWidget.h"
#include "ui_PostWidget.h"
#include "RunProcess.h"
#include "CalculateThread.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKCore/FITKThreadPool.h"

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

        QString shPath = creatStartParaViewSh(workDir, caseDir);

        //启动进程
        CalculateThread* thread = new CalculateThread();
        auto info = thread->getInfo();
        info->_cmd = QString("/bin/bash %1").arg(shPath);

        //获取线程池
        Core::FITKThreadPool* pool = Core::FITKThreadPool::getInstance();
        if (pool) {
            pool->execTask(thread);
        }
    }

    QString PostWidget::creatStartParaViewSh(QString workDir, QString caseDir)
    {
        //脚本生成
        QString shFilePath = workDir + "/startOpenParaFoam.sh";
        QFile file(shFilePath);
        // 打开文件进行写操作
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return "";
        // 创建一个 QTextStream 对象来写入文本
        QTextStream out(&file);
        // 写入字符串到文件
        out << QString("cd %1").arg(caseDir);
        out << QStringLiteral("\n");
        out << QString("paraFoam -builtin");
        // 关闭文件
        file.close();
        return shFilePath;
    }
}
