#include "ThermoWidget.h"
#include "ui_ThermoWidget.h"
#include "CalculateDriver.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramTaskManager.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"
#include "FITK_Kernel/FITKEasyParam/FITKEasyParamWidgetFactory.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"
#include "FITK_Kernel/FITKCore/FITKEnumTransformer.hpp"

#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFThermoPropManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFThermo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFThermoPhysicalEquationOfState.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFThermoPhysicalTransport.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFThermoPhysicalThermodynamics.h"

#include <QButtonGroup>
#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QLabel>
#include <QTabBar>
#include <QComboBox>
#include <QVBoxLayout>
#include <QSpacerItem>


namespace GUI
{
    Core::FITKParameter* equationofStateModelGetSubData(const QString & test, int type)
    {
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;
        //工厂动作
        phyFactory->setThermoEquationOfState(type, test);
        //获取辐射模型数据
        Interface::FITKOFThermo* thermo = phyData->getThermo();
        if (thermo == nullptr) return nullptr;
        Interface::FITKOFThermoPhysicalProp * thermoPhysicalProp = thermo->getThermoPropRegionByType(Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType(type));
        if (thermoPhysicalProp == nullptr) return nullptr;
        Interface::FITKOFAbsThermoPhysicalEquationOfState * equationOfState = thermoPhysicalProp->getEquationOfStatePara();
        return equationOfState->getEquationOfStatePara();
    }

    Core::FITKParameter* thermodynamicsModelGetSubData(const QString & test, int type)
    {
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;
        //工厂动作
        phyFactory->setThermoThermodynamics(type, test);
        //获取辐射模型数据
        Interface::FITKOFThermo* thermo = phyData->getThermo();
        if (thermo == nullptr) return nullptr;
        Interface::FITKOFThermoPhysicalProp * thermoPhysicalProp = thermo->getThermoPropRegionByType(Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType(type));
        if (thermoPhysicalProp == nullptr) return nullptr;
        Interface::FITKOFAbsThermoPhysicalThermodynamics * thermodynamics = thermoPhysicalProp->getThermodynamicsPara();
        return thermodynamics->getThermodynamicsPara();
    }

    Core::FITKParameter* transportModelGetSubData(const QString & test, int type)
    {
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;
        //工厂动作
        phyFactory->setThermoTransport(type, test);
        //获取辐射模型数据
        Interface::FITKOFThermo* thermo = phyData->getThermo();
        if (thermo == nullptr) return nullptr;
        Interface::FITKOFThermoPhysicalProp * thermoPhysicalProp = thermo->getThermoPropRegionByType(Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType(type));
        if (thermoPhysicalProp == nullptr) return nullptr;
        Interface::FITKOFAbsThermoPhysicalTransport * transport = thermoPhysicalProp->getTransportPara();
        return transport->getTransportPara();
    }

    ThermoWidget::ThermoWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUICalculateWidgetBase(oper,parent)
    {
        _ui = new Ui::ThermoWidget();
        _ui->setupUi(this);
        //获取数据
        if (_physicsData)
            _thermoObj = _physicsData->getThermo();
        init();
    }

    ThermoWidget::~ThermoWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void ThermoWidget::init()
    {
        if (_thermoObj == nullptr) return;
        //初始化热物理属性
        Core::FITKEnumTransfer<Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType> enumTransfer;
        QList<Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType> typeList = _thermoObj->getThermoPropRegionsType();
        for (Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType t : typeList)
        {
            bool isOK = false;
            QString strEnum = enumTransfer.toString(t, isOK);
            if (!isOK) continue;
            //创建窗口和垂直分布
            QWidget* w = new QWidget(this);
            _ui->tabWidget_thermo->addTab(w, strEnum);
            this->initTabWidget(w, t);
        }
        _ui->tabWidget_thermo->setCurrentIndex(0);
    }

    void ThermoWidget::showEvent(QShowEvent * event)
    {
        Q_UNUSED(event);
        //int width = _ui->tabWidget->width();
        //int tabCount = _ui->tabWidget->count();
        //int tabWidth = width / tabCount;
        //_ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void ThermoWidget::resizeEvent(QResizeEvent * event)
    {
        Q_UNUSED(event);
        //int width = _ui->tabWidget->width();
        //int tabCount = _ui->tabWidget->count();
        //int tabWidth = width / tabCount;
        //_ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void ThermoWidget::initTabWidget(QWidget * w, int type)
    {
        if (!_physicsManager || !_thermoObj) return;
        Interface::FITKOFThermoPropManager * thermoPropMgr = _physicsManager->getThermoPropManager();
        if (!thermoPropMgr) return;
        //获取模型数据
        Interface::FITKOFThermoPhysicalProp * thermoPhyProp = _thermoObj->getThermoPropRegionByType(Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType(type));
        if (!thermoPhyProp) return;
        Core::FITKParameter * speciePara = thermoPhyProp->getSpeciePara();
        Interface::FITKOFAbsThermoPhysicalEquationOfState * equationofState = thermoPhyProp->getEquationOfStatePara();
        Interface::FITKOFAbsThermoPhysicalThermodynamics * thermodynamics = thermoPhyProp->getThermodynamicsPara();
        Interface::FITKOFAbsThermoPhysicalTransport * transport = thermoPhyProp->getTransportPara();
        if (equationofState == nullptr || thermodynamics == nullptr || transport == nullptr) return;
        QStringList equationOfStateOptions = thermoPropMgr->filterEquationOfState(_physicsData->getSolver()->getSolverType());
        QStringList thermodynamicsOptions = thermoPropMgr->filterThermodynamics(equationofState->getDataObjectName(), _physicsData->getSolver()->getSolverType());
        QStringList transportOptions = thermoPropMgr->filterTransport(equationofState->getDataObjectName(), thermodynamics->getDataObjectName(), _physicsData->getSolver()->getSolverType());
        //初始化界面
        QVBoxLayout* pLayout = new QVBoxLayout(w);
        w->setLayout(pLayout);
        //Equation of State下拉框
        QHBoxLayout* comboBoxLayout = new QHBoxLayout(w);
        QLabel* label = new QLabel(w);
        label->setText(tr("Equation of State"));
        comboBoxLayout->addWidget(label);
        QComboBox* comboBox = new QComboBox(w);
        comboBox->setObjectName("comboBox_EquationofState");
        comboBox->addItems(equationOfStateOptions);
        comboBox->setCurrentText(equationofState->getDataObjectName());
        comboBoxLayout->addWidget(comboBox);
        connect(comboBox, &QComboBox::currentTextChanged, [type, this](QString text) {
            Core::FITKParameter * para = equationofStateModelGetSubData(text, type);
            this->initLayout(type, 2, para);
        });
        pLayout->addLayout(comboBoxLayout);
        //Thermodynamics 下拉框
        comboBoxLayout = new QHBoxLayout(w);
        label = new QLabel(w);
        label->setText(tr("Thermodynamics"));
        comboBoxLayout->addWidget(label);
        comboBox = new QComboBox(w);
        comboBox->setObjectName("comboBox_Thermodynamics");
        comboBox->addItems(thermodynamicsOptions);
        comboBox->setCurrentText(thermodynamics->getDataObjectName());
        comboBoxLayout->addWidget(comboBox);
        connect(comboBox, &QComboBox::currentTextChanged, [type, this](QString text) {

        });
        pLayout->addLayout(comboBoxLayout);
        //Transport 下拉框
        comboBoxLayout = new QHBoxLayout(w);
        label = new QLabel(w);
        label->setText(tr("Transport"));
        comboBoxLayout->addWidget(label);
        comboBox = new QComboBox(w);
        comboBox->setObjectName("comboBox_Transport");
        comboBox->addItems(transportOptions);
        comboBox->setCurrentText(transport->getDataObjectName());
        comboBoxLayout->addWidget(comboBox);
        connect(comboBox, &QComboBox::currentTextChanged, [type, this](QString text) {
            
        });
        pLayout->addLayout(comboBoxLayout);
        //Specie
        this->initSpecieWidget(speciePara, pLayout);
        //Thermodynamics
        QVBoxLayout* layoutNew = new QVBoxLayout(this);
        _thermodynamicsVBoxLayout.insert(type, layoutNew);
        pLayout->addLayout(layoutNew);
        this->initLayout(type, 0, thermodynamics->getThermodynamicsPara());
        //Transport
        layoutNew = new QVBoxLayout(this);
        _transportVBoxLayout.insert(type, layoutNew);
        pLayout->addLayout(layoutNew);
        this->initLayout(type, 1, transport->getTransportPara());
        //Equation of State
        layoutNew = new QVBoxLayout(this);
        _equationOfStateVBoxLayout.insert(type, layoutNew);
        pLayout->addLayout(layoutNew);
        this->initLayout(type, 2, equationofState->getEquationOfStatePara());
        QSpacerItem* verticalSpacer = new QSpacerItem(20, 45, QSizePolicy::Minimum, QSizePolicy::Expanding);
        pLayout->addItem(verticalSpacer);
    }

    void ThermoWidget::initSpecieWidget(Core::FITKParameter * para, QVBoxLayout * pLayout)
    {
        //Specie
        if (!para || para->getParameter().isEmpty())return;
        QLabel* label = new QLabel;
        label->setText("Specie");
        pLayout->addWidget(label);
        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        pLayout->addWidget(line);
        //初始化参数
        QList<Core::FITKAbstractEasyParam*> paramList = para->getParameter();
        for (auto value : paramList) {
            QWidget* wPara = new Core::FITKWidgetComLine(value, this);
            if (wPara == nullptr)continue;
            pLayout->addWidget(wPara);
        }
    }

    void ThermoWidget::initLayout(int type, int modelType, Core::FITKParameter * para)
    {
        //获取分布数据
        QString modelName;
        QVBoxLayout* layout = nullptr;
        if (modelType == 0)
        {
            layout = _thermodynamicsVBoxLayout[type];
            modelName = "Thermodynamics";
        }
        else if (modelType == 1)
        {
            layout = _transportVBoxLayout[type];
            modelName = "Transport";
        }
        else if (modelType == 2)
        {
            layout = _equationOfStateVBoxLayout[type];
            modelName = "Equation of State";
        }
        if (layout == nullptr) return;
        //清理布局数据
        while (QLayoutItem* item = layout->takeAt(0))
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();

            delete item;
        }

        if (!para || para->getDataCount() == 0) return;
        //初始化参数界面
        QLabel* label = new QLabel;
        label->setText(modelName);
        layout->addWidget(label);
        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);
        QList<Core::FITKAbstractEasyParam*> paramList = para->getParameter();
        for (auto value : paramList) {
            QWidget* wPara = new Core::FITKWidgetComLine(value, this);
            if (wPara == nullptr)continue;
            layout->addWidget(wPara);
        }
    }

}
