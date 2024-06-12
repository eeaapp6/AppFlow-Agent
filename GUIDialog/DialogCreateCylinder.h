#ifndef DIALOGCREATECYLINDER_H
#define DIALOGCREATECYLINDER_H

#include "GUIDialogAPI.h"
#include "FITK_Kernal/FITKCore/FITKActionOperator.h"
#include <QDialog>

namespace Ui {
	class CreateCylinderDialog;
}

namespace GUI {
	class GUIDIALOGAPI CreateCylinderDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit CreateCylinderDialog(Core::FITKActionOperator* oper, QWidget *parent = nullptr);
		~CreateCylinderDialog();

	protected:
		void accept() override;
		void reject() override;


	private slots:
		void on_rb_customAxis_toggled(bool checked);

	private:
		Ui::CreateCylinderDialog *m_Ui{};
		Core::FITKActionOperator* m_Oper{};
	};
}

#endif // DIALOGCREATECYLINDER_H
