#include "SetupWidget.h"
#include "ui_SetupWidget.h"

#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"

#include <QButtonGroup>

#define SetupTypePos "setupWidgetSolverTypePost"
#define SetupSolverPos Qt::UserRole+1
//#define SetupType Interface::FITKOFPostProcessEnum::FITKOFSolverFiltersType

namespace GUI
{
    SetupWidget::SetupWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _ui = new Ui::SetupWidget();
        _ui->setupUi(this);

        init();

        //_solverFactory = dynamic_cast<Interface::FITKFlowSolverProcessFactory*>
        //    (FITKAPP->getComponents()->getComponentByName("FITKFlowSolverProcess"));
    }

    SetupWidget::~SetupWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void SetupWidget::init()
    {
        //_ui->pushButton_SteadyState->setProperty(SetupTypePos, QVariant::fromValue(SetupType::SteadyState));
        //_ui->pushButton_SteadyState->setCheckable(true);
        //_ui->pushButton_Transient->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Transient));
        //_ui->pushButton_Transient->setCheckable(true);
        //_ui->pushButton_Compressible->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Compressible));
        //_ui->pushButton_Compressible->setCheckable(true);
        //_ui->pushButton_Incompressible->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Incompressible));
        //_ui->pushButton_Incompressible->setCheckable(true);
        //_ui->checkBox_Turbulence->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Turbulences));
        //_ui->checkBox_HeatTransfer->setProperty(SetupTypePos, QVariant::fromValue(SetupType::HeatTransfer));
        //_ui->checkBox_Radiation->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Radiation));
        //_ui->checkBox_Buoyancy->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Buoyancy));
        //_ui->checkBox_MRF->setProperty(SetupTypePos, QVariant::fromValue(SetupType::MRF));
        //_ui->checkBox_SRF->setProperty(SetupTypePos, QVariant::fromValue(SetupType::SRF));
        //_ui->checkBox_Porosity->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Porosity));
        //_ui->checkBox_Multiphase->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Multiphase));
        //_ui->checkBox_Species->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Spcies));
        //_ui->checkBox_Lagrangian->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Lagrangian));
        //_ui->checkBox_DynamicMesh->setProperty(SetupTypePos, QVariant::fromValue(SetupType::DynamicMesh));
        //_ui->checkBox_UserDefined->setProperty(SetupTypePos, QVariant::fromValue(SetupType::UserDefined));

        //_ui->tableWidget->setRowCount(0);
        //_ui->tableWidget->setColumnCount(1);
        ////自适应
        //_ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ////隐藏行表头
        //_ui->tableWidget->verticalHeader()->setVisible(false);
        ////隐藏列表头
        //_ui->tableWidget->horizontalHeader()->setVisible(false);
        ////设置不可编辑
        //_ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        //initCurrentType();
        //initSetupType();
        //updateTableWidget();

        //QList<QAbstractButton*> radioList = this->findChildren<QAbstractButton*>();
        //for (auto radio : radioList) {
        //    if (radio == nullptr)continue;
        //    connect(radio, SIGNAL(clicked()), this, SLOT(slotTypeSelect()));
        //}
    }

    void SetupWidget::updateTableWidget()
    {
        //_ui->tableWidget->clear();
        //_ui->tableWidget->setRowCount(0);

        //for (int i = 0; i < _types.size(); i++) {
        //    Interface::FITKOFPostProcessEnum::FITKOFSolverType type = _types[i];
        //    QString name = typeToName(type);
        //    QTableWidgetItem* item = new QTableWidgetItem(name);
        //    item->setData(SetupSolverPos, QVariant::fromValue(type));
        //    _ui->tableWidget->insertRow(i);
        //    _ui->tableWidget->setItem(i, 0, item);
        //}
    }

    void SetupWidget::on_pushButton_Select_clicked()
    {
        //if (_solverFactory == nullptr)return;
        //QTableWidgetItem* currentItem = _ui->tableWidget->currentItem();
        //if (currentItem == nullptr)return;
        //_ui->label_CurrentSolver->setText(currentItem->text());
        //_solverFactory->setSolverType(currentItem->data(SetupSolverPos).value<Interface::FITKOFSolverTypeEnum::FITKOFSolverType>());

        //if (_oper) {
        //    _oper->execProfession();
        //}
    }

    QString SetupWidget::typeToName(Interface::FITKOFSolverTypeEnum::FITKOFSolverType type)
    {
        QString name = "";
        switch (type) {
        case Interface::FITKOFSolverTypeEnum::FITKOFSolverType::SolverNone: break;
        case Interface::FITKOFSolverTypeEnum::FITKOFSolverType::SIMPLE: name = tr("SIMPLE"); break;
        case Interface::FITKOFSolverTypeEnum::FITKOFSolverType::INTER: name = tr("Inter"); break;
        }
        return name;
    }

    void SetupWidget::initSetupType()
    {
        //_types.append(Interface::FITKOFPostProcessEnum::SIMPLE);
        //_types.append(Interface::FITKOFPostProcessEnum::Inter);
    }

    void SetupWidget::initCurrentType()
    {
        //Interface::FITKOFCasePhysicsData* solver = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFCasePhysicsData>();
        //if (solver == nullptr)return;
        //QString name = typeToName(solver->getSolverType());
        //_ui->label_CurrentSolver->setText(name);
    }

    void SetupWidget::slotTypeSelect()
    {
        //QPushButton* senderObj = dynamic_cast<QPushButton*>(sender());
        //if (senderObj == _ui->pushButton_SteadyState) {
        //    bool isChecked = _ui->pushButton_SteadyState->isChecked();
        //    if (isChecked == true) {
        //        _ui->pushButton_Transient->setChecked(false);
        //    }
        //}
        //else if (senderObj == _ui->pushButton_Transient) {
        //    bool isChecked = _ui->pushButton_Transient->isChecked();
        //    if (isChecked == true) {
        //        _ui->pushButton_SteadyState->setChecked(false);
        //    }
        //}
        //else if (senderObj == _ui->pushButton_Incompressible) {
        //    bool isChecked = _ui->pushButton_Incompressible->isChecked();
        //    if (isChecked == true) {
        //        _ui->pushButton_Compressible->setChecked(false);
        //    }
        //}
        //else if (senderObj == _ui->pushButton_Compressible) {
        //    bool isChecked = _ui->pushButton_Compressible->isChecked();
        //    if (isChecked == true) {
        //        _ui->pushButton_Incompressible->setChecked(false);
        //    }
        //}

        //if (_solverFactory == nullptr)return;
        //_types.clear();
        //QList<SetupType> typeList = {};
        //QList<QAbstractButton*> radioList = this->findChildren<QAbstractButton*>();
        //for (auto radio : radioList) {
        //    if (radio == nullptr)continue;
        //    if (radio->isChecked()) {
        //        typeList.append(radio->property(SetupTypePos).value<SetupType>());
        //    }
        //}

        //_types = _solverFactory->getSolverTypeByFilters(typeList);

        //updateTableWidget();
    }
}

