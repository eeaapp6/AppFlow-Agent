#include "DialogCreateSphere.h"
#include "ui_DialogCreateSphere.h"
#include "FITK_Interface/FITKInterfaceOCC/FITKAbstractOCCModel.h"
#include "FITK_Interface/FITKInterfaceOCC/FITKOCCModelSphere.h"
#include "FITK_Kernal/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernal/FITKAppFramework/FITKComponentFactory.h"
#include "FITK_Kernal/FITKAppFramework/FITKComponentInterface.h"
#include "FITK_Kernal/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernal/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Kernal/FITKCore/FITKOperatorRepo.h"
#include "OperatorsInterface/GraphEventOperator.h"

namespace GUI {
	CreateSphereDialog::CreateSphereDialog(Core::FITKActionOperator* oper, QWidget *parent) :
		QDialog(parent), m_Oper(oper),
		m_Ui(new Ui::CreateSphereDialog)
	{
		m_Ui->setupUi(this);
		m_Ui->le_x->setText("0.0");
		m_Ui->le_y->setText("0.0");
		m_Ui->le_z->setText("0.0");

		m_Ui->le_radius->setText("10.0");

		auto geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
		if (geometryData == nullptr) return;
		m_Ui->le_name->setText(geometryData->checkName(QString("Sphere-%1").arg(geometryData->getDataCount() + 1)));
	}

	CreateSphereDialog::~CreateSphereDialog()
	{
		delete m_Ui;
	}

	void CreateSphereDialog::accept()
	{
		auto sphere = new Interface::FITKOCCModelSphere;

		auto locX = m_Ui->le_x->text().toDouble();
		auto locY = m_Ui->le_y->text().toDouble();
		auto locZ = m_Ui->le_z->text().toDouble();

		auto radius = m_Ui->le_radius->text().toDouble();

		double point1[3] = { locX, locY, locZ };

		sphere->setLocation(point1);
		sphere->setRadius(radius);

		sphere->update();
		auto geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
		if (geometryData == nullptr) return;

		geometryData->appendDataObj(sphere);

		EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
		if (graphOper)
		{
			graphOper->updateGraph(sphere->getDataObjectID());
		}

		QDialog::accept();
		this->close();
	}

	void CreateSphereDialog::reject()
	{
		QDialog::reject();
		this->close();
	}

}