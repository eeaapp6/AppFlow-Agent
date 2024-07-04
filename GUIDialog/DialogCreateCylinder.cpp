#include "DialogCreateCylinder.h"
#include "ui_DialogCreateCylinder.h"
#include "FITK_Interface/FITKInterfaceOCC/FITKAbstractOCCModel.h"
#include "FITK_Interface/FITKInterfaceOCC/FITKOCCModelCylinder.h"
#include "FITK_Kernal/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernal/FITKAppFramework/FITKComponentFactory.h"
#include "FITK_Kernal/FITKAppFramework/FITKComponentInterface.h"
#include "FITK_Kernal/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernal/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Kernal/FITKCore/FITKOperatorRepo.h"
#include "OperatorsInterface/GraphEventOperator.h"

namespace GUI {
	CreateCylinderDialog::CreateCylinderDialog(Core::FITKActionOperator* oper, QWidget *parent) :
		QDialog(parent), m_Oper(oper),
		m_Ui(new Ui::CreateCylinderDialog)
	{
		m_Ui->setupUi(this);

		m_Ui->le_x->setText("0.0");
		m_Ui->le_y->setText("0.0");
		m_Ui->le_z->setText("0.0");

		m_Ui->le_radius->setText("5.0");
		m_Ui->le_height->setText("10.0");
		m_Ui->rb_zAxis->setChecked(true);

		m_Ui->le_axisx->setText("0.0");
		m_Ui->le_axisy->setText("0.0");
		m_Ui->le_axisz->setText("1.0");

		on_rb_customAxis_toggled(false);

		auto geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
		if (geometryData == nullptr) return;
		m_Ui->le_name->setText(geometryData->checkName(QString("Cylinder-%1").arg(geometryData->getDataCount() + 1)));
	}

	CreateCylinderDialog::~CreateCylinderDialog()
	{
		delete m_Ui;
	}

	void CreateCylinderDialog::accept()
	{
		auto cylinder = new Interface::FITKOCCModelCylinder;

		auto locX = m_Ui->le_x->text().toDouble();
		auto locY = m_Ui->le_y->text().toDouble();
		auto locZ = m_Ui->le_z->text().toDouble();

		auto radius = m_Ui->le_radius->text().toDouble();
		auto height = m_Ui->le_height->text().toDouble();

		double point1[3] = { locX, locY, locZ };

		cylinder->setLocation(point1);
		cylinder->setRadius(radius);
		cylinder->setLength(height);

		double dir[3] = {0.0, 0.0, 0.0};
		if (m_Ui->rb_xAxis->isChecked()) {
			dir[0] = 1.0;
		}
		else if (m_Ui->rb_yAxis->isChecked()) {
			dir[1] = 1.0;
		}
		else if (m_Ui->rb_zAxis->isChecked()) {
			dir[2] = 1.0;
		}
		else if (m_Ui->rb_customAxis->isChecked()) {
			dir[0] = m_Ui->le_axisx->text().toDouble();
			dir[1] = m_Ui->le_axisy->text().toDouble();
			dir[2] = m_Ui->le_axisz->text().toDouble();
		}
		if (qFuzzyIsNull(dir[0]) && qFuzzyIsNull(dir[1]) && qFuzzyIsNull(dir[2])) return;
		cylinder->setDirection(dir);

		cylinder->update();
		auto geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
		if (geometryData == nullptr) return;

		geometryData->appendDataObj(cylinder);

		EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
		if (graphOper)
		{
			graphOper->updateGraph(cylinder->getDataObjectID());
		}

		QDialog::accept();
		this->close();

		if (m_Oper != nullptr)
			m_Oper->execProfession();
	}

	void CreateCylinderDialog::reject()
	{
		QDialog::reject();
		this->close();
	}

	void CreateCylinderDialog::on_rb_customAxis_toggled(bool checked)
	{
		m_Ui->le_axisx->setVisible(checked);
		m_Ui->le_axisy->setVisible(checked);
		m_Ui->le_axisz->setVisible(checked);
	}
}

