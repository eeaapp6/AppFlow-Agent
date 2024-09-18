#include "PostWidget.h"
#include "ui_PostWidget.h"
#include "CalculateDriver.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKWorkBenchHandler.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramTaskManager.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"
#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"

#include <QFile>
#include <QTextStream>

Q_DECLARE_METATYPE(GUI::PostExportType)

namespace GUI
{
    PostWidget::PostWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _ui = new Ui::PostWidget();
        _ui->setupUi(this);

        init();
    }

    PostWidget::~PostWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void PostWidget::init()
    {
        //工作路径获取
        if (FITKAPP->getAppSettings()) {
            _workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (_workDir.isEmpty()) _workDir = QApplication::applicationDirPath() + "/../WorkDir";
        _caseDir = _workDir + "/case";
        Core::CreateDir(_caseDir);

        _ui->comboBox_Export->addItem(tr("VTK"), QVariant::fromValue(PostExportType::Post_VTK));
    }

    void GUI::PostWidget::on_pushButton_ParaView_clicked()
    {
        //创建paraView打开时间步文件
        QString foamFile = creatStartParaViewFile();
        if (foamFile.isEmpty())return;

        auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
        auto proGramManager = FITKAPP->getProgramTaskManager();
        AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
        QStringList args;
        args << "--case" << foamFile;
        info->setArgs(args);
        auto progam = proGramManager->createProgram(1, "CalculateDriver", info);
        if (!progam) return;
        CalculateDriver* calDriver = dynamic_cast<CalculateDriver*>(progam);
        if (calDriver)calDriver->setExecProgram("paraview");
        //启动进程
        progam->start();
    }

    QString PostWidget::creatStartParaViewFile()
    {
        //创建case.foam文件用于paraView启动，查看后处理结果
        QString foamFile = _caseDir + "/case.foam";

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

void GUI::PostWidget::on_pushButton_Post_clicked()
{

}

void GUI::PostWidget::on_pushButton_Export_clicked()
{
    auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
    auto proGramManager = app->getProgramTaskManager();
    AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
    QStringList args;
    args << "-ascii" << "-case" << _caseDir;
    info->setArgs(args);
    auto progam = proGramManager->createProgram(1, "CalculateDriver", info);
    if (!progam) return;
    CalculateDriver* calDriver = dynamic_cast<CalculateDriver*>(progam);
    if (calDriver) {
        PostExportType type = _ui->comboBox_Export->currentData().value<PostExportType>();
        switch (type) {
        case GUI::PostExportType::Post_VTK: {
            calDriver->setExecProgram("foamToVTK");
            break;
        }
        }
    }
    //启动进程
    progam->start();
    //写出到wb配置文件
    connect(progam, &AppFrame::FITKAbstractProgramerDriver::sig_Finish, this, [&] {
        if (!FITKAPP->workingInWorkBench()) return;
        AppFrame::FITKWorkBenchHandler* h = FITKAPP->getWorkBenchHandler();
        h->clearOutputInfo();
        AppFrame::IOFileInfo info;
        info._name = "case_";
        info._path = _caseDir+"/VTK";
        info._suffix = "vtk";
        info._describe = "OpenFOAM_Result";
        info._isFile = false;
        h->appendOutputInfo(info);
    });

}
