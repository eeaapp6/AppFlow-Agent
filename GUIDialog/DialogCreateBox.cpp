#include "DialogCreateBox.h"
#include "ui_DialogCreateBox.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelBox.h"
#include "FITK_Kernal/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernal/FITKAppFramework/FITKComponentFactory.h"
#include "FITK_Kernal/FITKAppFramework/FITKComponentInterface.h"
#include "FITK_Kernal/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernal/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Kernal/FITKCore/FITKOperatorRepo.h"
#include "OperatorsInterface/GraphEventOperator.h"

namespace GUI {
	CreateBoxDialog::CreateBoxDialog(Core::FITKActionOperator* oper, QWidget *parent) :
		QDialog(parent), m_Oper(oper),
		m_Ui(new Ui::CreateBoxDialog)
	{
		m_Ui->setupUi(this);
		m_Ui->le_x->setText("0.0");
		m_Ui->le_y->setText("0.0");
		m_Ui->le_z->setText("0.0");

		m_Ui->le_length->setText("10.0");
		m_Ui->le_width->setText("10.0");
		m_Ui->le_height->setText("10.0");


		auto geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
		if (geometryData == nullptr) return;
		m_Ui->le_name->setText(geometryData->checkName(QString("Box-%1").arg(geometryData->getDataCount() + 1)));
	}

	CreateBoxDialog::~CreateBoxDialog()
	{
		delete m_Ui;
	}

	void CreateBoxDialog::accept()
	{
        auto fac = Interface::FITKInterfaceGeometryFactory::getInstance();

        auto box = fac->createCommandT<Interface::FITKAbsGeoModelBox>
            (Interface::FITKGeoEnum::FGTBox);
        if (box == nullptr) return;

		auto locX = m_Ui->le_x->text().toDouble();
		auto locY = m_Ui->le_y->text().toDouble();
		auto locZ = m_Ui->le_z->text().toDouble();

		auto length = m_Ui->le_length->text().toDouble();
		auto width = m_Ui->le_width->text().toDouble();
		auto height = m_Ui->le_height->text().toDouble();

		double point1[3] = { locX, locY, locZ };
		double point2[3] = { locX + length, locY + width, locZ + height };

		box->setPoint1(point1);
		box->setPoint2(point2);
		box->update();

		auto geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
		if (geometryData == nullptr) return;

		geometryData->appendDataObj(box);

		EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
		if (graphOper)
		{
			graphOper->updateGraph(box->getDataObjectID());
		}

		QDialog::accept();
		this->close();

		if (m_Oper != nullptr)
			m_Oper->execProfession();
	}

	void CreateBoxDialog::reject()
	{
		QDialog::reject();
		this->close();
	}

}