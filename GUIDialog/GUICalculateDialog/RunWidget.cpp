#include "RunWidget.h"
#include "ui_RunWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"

#include <QButtonGroup>
#include <QProcess>

#define CPUType "CPUType"
Q_DECLARE_METATYPE(GUI::RunCPUType)

namespace GUI
{
    static RunCPUType _currentCUPType = RunCPUType::Serial;
    static int _currentCUPNum = 4;

    RunWidget::RunWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        QWidget(parent), _oper(oper)
    {
        _ui = new Ui::RunWidget();
        _ui->setupUi(this);

        init();
    }

    RunWidget::~RunWidget()
    {
        if (_ui)delete _ui;
    }

    void RunWidget::init()
    {
        initCPU();
    }

    void RunWidget::slotCPUChange(QAbstractButton * button)
    {
        if (button == nullptr)return;
        _currentCUPType = button->property(CPUType).value<RunCPUType>();
        updateCPU();
    }

    void RunWidget::on_spinBox_NumOfPro_valueChanged(int arg1)
    {
        _currentCUPNum = arg1;
        updateCPU();
    }

    void RunWidget::on_pushButton_Run_clicked()
    {
        QString sh = "";
        switch (_currentCUPType){
        case GUI::RunCPUType::Serial:break;
        case GUI::RunCPUType::Parallel: {
            sh += QString("mpirun -np %1").arg(_currentCUPNum);
            break;
        }
        }
        sh += "simpleFoam -case /home/baguijun/public/openFOAMTest/pipe";

        //进程测试
        QProcess* process = new QProcess();
        process->start(sh);
        connect(process, &QProcess::readyReadStandardOutput, [&]()
        {
            QString message = process->readAllStandardOutput();
            emit FITKAPP->getSignalTransfer()->outputMessageSig(4, message);
        });
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
        switch (_currentCUPType){
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
}
