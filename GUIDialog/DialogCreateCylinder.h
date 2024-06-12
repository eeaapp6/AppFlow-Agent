#ifndef DIALOGCREATECYLINDER_H
#define DIALOGCREATECYLINDER_H

#include "GUIDialogAPI.h"
#include <QDialog>

namespace Ui {
class CreateCylinderDialog;
}

namespace GUI {
	class GUIDIALOGAPI CreateCylinderDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit CreateCylinderDialog(QWidget *parent = nullptr);
		~CreateCylinderDialog();

	protected:
		void accept() override;
		void reject() override;


	private:
		Ui::CreateCylinderDialog *m_Ui{};
	};
}

#endif // DIALOGCREATECYLINDER_H
