#ifndef DIALOGCREATESPHERE_H
#define DIALOGCREATESPHERE_H

#include "GUIDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include <QDialog>

namespace Ui {
	class CreateSphereDialog;
}

namespace GUI {
	class GUIDIALOGAPI CreateSphereDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit CreateSphereDialog(Core::FITKActionOperator* oper, QWidget *parent = nullptr);
		~CreateSphereDialog();

	protected:
		void accept() override;
		void reject() override;

	private:
		Ui::CreateSphereDialog *m_Ui{};
		Core::FITKActionOperator* m_Oper{};
	};
}

#endif // DIALOGCREATESPHERE_H
