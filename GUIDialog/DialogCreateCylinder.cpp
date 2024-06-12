#include "DialogCreateCylinder.h"
#include "ui_DialogCreateCylinder.h"

namespace GUI {
	CreateCylinderDialog::CreateCylinderDialog(QWidget *parent) :
		QDialog(parent),
		m_Ui(new Ui::CreateCylinderDialog)
	{
		m_Ui->setupUi(this);
	}

	CreateCylinderDialog::~CreateCylinderDialog()
	{
		delete m_Ui;
	}

	void CreateCylinderDialog::accept()
	{
		QDialog::accept();
		this->close();
	}

	void CreateCylinderDialog::reject()
	{
		QDialog::reject();
		this->close();
	}

}