#include "DialogCreateSphere.h"
#include "ui_DialogCreateSphere.h"


namespace GUI {
	CreateSphereDialog::CreateSphereDialog(QWidget *parent) :
		QDialog(parent),
		m_Ui(new Ui::CreateSphereDialog)
	{
		m_Ui->setupUi(this);
	}

	CreateSphereDialog::~CreateSphereDialog()
	{
		delete m_Ui;
	}

	void CreateSphereDialog::accept()
	{
		QDialog::accept();
		this->close();
	}

	void CreateSphereDialog::reject()
	{
		QDialog::reject();
		this->close();
	}

}