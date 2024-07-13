#ifndef DIALOGCREATEBOX_H
#define DIALOGCREATEBOX_H

#include "GUIDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include <QDialog>

namespace Ui {
	class CreateBoxDialog;
}

namespace GUI {
	class GUIDIALOGAPI CreateBoxDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit CreateBoxDialog(Core::FITKActionOperator* oper, QWidget *parent = nullptr);
		~CreateBoxDialog();

	protected:
		void accept() override;
		void reject() override;

	private:
		Ui::CreateBoxDialog *m_Ui{};
		Core::FITKActionOperator* m_Oper{};
	};
}

#endif // DIALOGCREATEBOX_H
