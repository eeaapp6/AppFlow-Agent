#include "RunWidget.h"
#include "ui_RunWidget.h"
#include "RunProcess.h"
#include "CompCalLineWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"
#include "FITK_Component/FITKOFDictWriter/FITKOFDictWriterIO.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRunControl.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"

#include <QButtonGroup>
#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QTabBar>

#define CPUType "CPUType"
Q_DECLARE_METATYPE(GUI::RunCPUType)

namespace GUI
{
    static RunCPUType _currentCUPType = RunCPUType::Serial;
    static int _currentCUPNum = 4;
    static RunProcess* _currentPro = nullptr;

    RunWidget::RunWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUICalculateWidgetBase(oper,parent)
    {
        _ui = new Ui::RunWidget();
        _ui->setupUi(this);

        init();
    }

    RunWidget::~RunWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void RunWidget::init()
    {
        if (_physicsData) {
            _runConObj = _physicsData->getRunControl();
        }
        updataTime();
        updateOutput();
        initCPU();
        setRunType(_currentPro);
    }

    void RunWidget::showEvent(QShowEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void RunWidget::resizeEvent(QResizeEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void RunWidget::slotCPUChange(QAbstractButton * button)
    {
        if (button == nullptr)return;
        _currentCUPType = button->property(CPUType).value<RunCPUType>();
        updateCPU();
    }

    void RunWidget::slotProcessFinish()
    {
        if (_currentPro) {
            delete _currentPro;
            _currentPro = nullptr;
        }

        setRunType(_currentPro);
    }

    void RunWidget::on_spinBox_NumOfPro_valueChanged(int arg1)
    {
        _currentCUPNum = arg1;
        updateCPU();
    }

    void RunWidget::on_pushButton_Stop_clicked()
    {
        if (_currentPro) {
            _currentPro->kill();
        }
        slotProcessFinish();
    }

    void RunWidget::on_pushButton_Run_clicked()
    {
        //工作路径获取
        QString workDir = "";
        if (FITKAPP->getAppSettings()) {
            workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (workDir.isEmpty()) workDir = QApplication::applicationDirPath() + "/../WorkDir";
        QString caseDir = workDir + "/case";

        //清理字典文件
        if (!clearCasePath(caseDir))return;
        //写出字典文件
        if (!writeCase(caseDir))return;
        //写出启动脚本
        QString shPath = creatStartSh(workDir, caseDir);
        if (shPath.isEmpty())return;

        //启动进程
        if (_currentPro) {
            _currentPro->kill();
        }
        _currentPro = new RunProcess();
        setRunType(_currentPro);

        //进程结束信号处理
        connect(_currentPro, &RunProcess::sigFinish, [=]() {
            if (_currentPro) {
                delete _currentPro;
                _currentPro = nullptr;
            }
            if (_ui) setRunType(_currentPro);
        });
        _currentPro->start("/bin/bash " + shPath);
    }

    void RunWidget::updataTime()
    {
        if (_runConObj == nullptr)return;
        Interface::FITKAbstractParameter* tiemPara = _runConObj->getTimeControl();
        for (auto data : tiemPara->getParameter()) {
            if(data == nullptr)continue;
            QWidget* widget = nullptr;
            if (data->getDataType() == Interface::FlowDataType::FLowDataBoolGroup) {
                widget = CompCalLineWidget::DataSwitchToWidget(data, this);
            }
            else {
                widget = new CompCalLineWidget(data, this);
            }
            _ui->verticalLayout_Time->addWidget(widget);
        }
    }

    void RunWidget::updateOutput()
    {
        if (_runConObj == nullptr)return;
        Interface::FITKAbstractParameter* outPara = _runConObj->getOutputControl();
        for (auto data : outPara->getParameter()) {
            if (data == nullptr)continue;
            QWidget* widget = nullptr;
            if (data->getDataType() == Interface::FlowDataType::FLowDataBoolGroup) {
                widget = CompCalLineWidget::DataSwitchToWidget(data, this);
            }
            else {
                widget = new CompCalLineWidget(data, this);
            }
            _ui->verticalLayout_Output->addWidget(widget);
        }
    }

    void RunWidget::initCPU()
    {
        QButtonGroup* group = new QButtonGroup(this);
        _ui->radioButton_serial->setProperty(CPUType, QVariant::fromValue(RunCPUType::Serial));
        _ui->radioButton_parallel->setProperty(CPUType, QVariant::fromValue(RunCPUType::Parallel));
        group->addButton(_ui->radioButton_serial);
        group->addButton(_ui->radioButton_parallel);
        connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotCPUChange(QAbstractButton*)));
        updateCPU();
    }

    void RunWidget::updateCPU()
    {
        switch (_currentCUPType) {
        case GUI::RunCPUType::Serial: {
            _ui->spinBox_NumOfPro->setEnabled(false);
            _ui->radioButton_serial->setChecked(true);
            break;
        }
        case GUI::RunCPUType::Parallel: {
            _ui->spinBox_NumOfPro->setEnabled(true);
            _ui->radioButton_parallel->setChecked(true);
            break;
        }
        }
        _ui->spinBox_NumOfPro->setValue(_currentCUPNum);
    }

    bool RunWidget::clearCasePath(QString casePath)
    {
        QDir dir(casePath);
        // 设置过滤器，只获取目录
        dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

        // 获取目录列表
        QStringList directories = dir.entryList();

        //清除非system与constant文件夹
        for (auto path : directories) {
            if(path == "system")continue;
            if(path == "constant")continue;
            Core::RemoveDir(casePath + "/" + path);
        }

        return true;
    }

    bool RunWidget::writeCase(QString casePath)
    {
        auto dicWriComp = FITKAPP->getComponents()->getComponentTByName<IO::FITKOFDictWriterIO>("IO::FITKOFDictWriterIO");
        if (dicWriComp == nullptr)return false;

        if (!dicWriComp->setFilePath(casePath)) {
            Core::CreateDir(casePath);
            dicWriComp->setFilePath(casePath);
        }
        dicWriComp->setPhysicsDictW();
        if (!dicWriComp->exec())return false;

        return true;
    }

    QString RunWidget::creatStartSh(QString workDir, QString caseDir)
    {
        //脚本生成
        QString sh = "";
        switch (_currentCUPType) {
        case GUI::RunCPUType::Serial:break;
        case GUI::RunCPUType::Parallel: {
            sh += QString("mpirun -np %1 ").arg(_currentCUPNum);
            break;
        }
        }
        sh += QString("simpleFoam -case %1").arg(caseDir);

        QString shFilePath = workDir + "/startOpenFoam.sh";
        QFile file(shFilePath);
        // 打开文件进行写操作
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return "";
        // 创建一个 QTextStream 对象来写入文本
        QTextStream out(&file);
        // 写入字符串到文件
        out << sh;
        // 关闭文件
        file.close();
        return shFilePath;
    }

    void RunWidget::setRunType(bool isRun)
    {
        if (isRun) {
            _ui->progressBar->show();
            _ui->pushButton_Run->setEnabled(false);
            _ui->pushButton_Stop->setEnabled(true);
        }
        else
        {
            _ui->progressBar->hide();
            _ui->pushButton_Run->setEnabled(true);
            _ui->pushButton_Stop->setEnabled(false);
        }
    }
}
