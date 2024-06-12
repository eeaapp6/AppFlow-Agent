#include "DialogCreateBox.h"
#include "ui_DialogCreateBox.h"

namespace GUI {
	CreateBoxDialog::CreateBoxDialog(QWidget *parent) :
		QDialog(parent),
		m_Ui(new Ui::CreateBoxDialog)
	{
		m_Ui->setupUi(this);

	}

	CreateBoxDialog::~CreateBoxDialog()
	{
		delete m_Ui;
	}

	void CreateBoxDialog::accept()
	{

		QDialog::accept();
		this->close();
	}

	void CreateBoxDialog::reject()
	{
		QDialog::reject();
		this->close();
	}

}