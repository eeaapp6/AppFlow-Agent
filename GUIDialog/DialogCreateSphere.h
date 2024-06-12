#ifndef DIALOGCREATESPHERE_H
#define DIALOGCREATESPHERE_H

#include "GUIDialogAPI.h"
#include <QDialog>

namespace Ui {
class CreateSphereDialog;
}

namespace GUI {
	class GUIDIALOGAPI CreateSphereDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit CreateSphereDialog(QWidget *parent = nullptr);
		~CreateSphereDialog();

	protected:
		void accept() override;
		void reject() override;

	private:
		Ui::CreateSphereDialog *m_Ui{};
	};
}

#endif // DIALOGCREATESPHERE_H
