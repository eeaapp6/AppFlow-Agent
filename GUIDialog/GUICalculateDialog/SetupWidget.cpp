#include "SetupWidget.h"
#include "ui_SetupWidget.h"

#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

//#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSetUpCase.h"
//#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractSolver.h"

#include <QButtonGroup>

#define SetupTypePos Qt::UserRole

namespace GUI
{
    SetupWidget::SetupWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _ui = new Ui::SetupWidget();
        _ui->setupUi(this);

        /*_setUpCase = Interface::FITKOFSetUpCase::getInstance();*/

        init();
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
        QButtonGroup* buttonGroup = new QButtonGroup(this);
        buttonGroup->addButton(_ui->radioButton_SteadyState);
        buttonGroup->addButton(_ui->radioButton_Transient);

        buttonGroup = new QButtonGroup(this);
        buttonGroup->addButton(_ui->radioButton_Incompressible);
        buttonGroup->addButton(_ui->radioButton_Compressible);

        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);
        //自适应
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //隐藏行表头
        _ui->tableWidget->verticalHeader()->setVisible(false);
        //隐藏列表头
        _ui->tableWidget->horizontalHeader()->setVisible(false);
        //设置不可编辑
        _ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        initCurrentType();
        initSetupType();
        updateTableWidget();
    }

    void SetupWidget::updateTableWidget()
    {
        _ui->tableWidget->clear();
        _ui->tableWidget->setRowCount(0);

        //for (int i = 0; i < _types.size(); i++) {
        //    Interface::FITKOFSolverEnum::FITKOFSolverType type = _types[i];
        //    QString name = typeToName(type);
        //    QTableWidgetItem* item = new QTableWidgetItem(name);
        //    item->setData(SetupTypePos, QVariant::fromValue(type));
        //    _ui->tableWidget->insertRow(i);
        //    _ui->tableWidget->setItem(i, 0, item);
        //}
    }

    void SetupWidget::on_radioButton_SteadyState_clicked()
    {

    }

    void SetupWidget::on_radioButton_Transient_clicked()
    {

    }

    void SetupWidget::on_radioButton_Incompressible_clicked()
    {

    }

    void SetupWidget::on_radioButton_Compressible_clicked()
    {

    }

    void SetupWidget::on_pushButton_Select_clicked()
    {
        //QTableWidgetItem* currentItem = _ui->tableWidget->currentItem();
        //if (currentItem == nullptr)return;
        //_ui->label_CurrentSolver->setText(currentItem->text());
        //_setUpCase->createSolver(currentItem->data(SetupTypePos).value<Interface::FITKOFSolverEnum::FITKOFSolverType>());

        //if (_oper) {
        //    _oper->execProfession();
        //}
    }

    //QString SetupWidget::typeToName(Interface::FITKOFPostProcessEnum::FITKOFSolverType type)
    //{
    //    QString name = "";
    //    switch (type) {
    //    case Interface::FITKOFPostProcessEnum::FITKOFSolverType::NoneSolver: break;
    //    case Interface::FITKOFPostProcessEnum::FITKOFSolverType::SIMPLE: name = tr("SIMPLE"); break;
    //    case Interface::FITKOFPostProcessEnum::FITKOFSolverType::Inter: name = tr("Inter"); break;
    //    }
    //    return name;
    //}

    void SetupWidget::initSetupType()
    {
        //_types.append(Interface::FITKOFPostProcessEnum::SIMPLE);
        //_types.append(Interface::FITKOFPostProcessEnum::Inter);
    }

    void SetupWidget::initCurrentType()
    {
        //QString name = "";
        //if (_setUpCase->getCurrentSolver()) {
        //    name = typeToName(_setUpCase->getCurrentSolver()->getSolverType());
        //}
        //_ui->label_CurrentSolver->setText(name);
    }
}

