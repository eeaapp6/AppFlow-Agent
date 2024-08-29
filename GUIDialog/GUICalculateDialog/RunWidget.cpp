#include "RunWidget.h"
#include "ui_RunWidget.h"
#include "RunProcess.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Component/FITKOFDictWriter/FITKOFDictWriterIO.h"

#include <QButtonGroup>
#include <QProcess>

#define CPUType "CPUType"
Q_DECLARE_METATYPE(GUI::RunCPUType)

namespace GUI
{
    static RunCPUType _currentCUPType = RunCPUType::Serial;
    static int _currentCUPNum = 4;
    static RunProcess* _currentPro = new RunProcess();

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
        auto dicWriComp = FITKAPP->getComponents()->getComponentTByName<IO::FITKOFDictWriterIO>("IO::FITKOFDictWriterIO");
        if (dicWriComp == nullptr)return;
        //工作路径获取
        QString workDir = "";
        if (FITKAPP->getAppSettings()) {
            workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (workDir.isEmpty()) workDir = QApplication::applicationDirPath() + "/../WorkDir";
        QString caseDir = workDir + "/case";
        dicWriComp->setFilePath(caseDir);
        dicWriComp->setPhysicsDictW(true);
        if (!dicWriComp->exec())return;

        QString sh = "";
        switch (_currentCUPType) {
        case GUI::RunCPUType::Serial:break;
        case GUI::RunCPUType::Parallel: {
            sh += QString("mpirun -np %1").arg(_currentCUPNum);
            break;
        }
        }
        sh += "simpleFoam -case /home/baguijun/public/openFOAMTest/pipe";

        if (_currentPro) {
            _currentPro->start(sh);
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
}
