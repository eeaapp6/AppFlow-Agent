#include "OperModelManager.h"
#include "GUIDialog/DialogCreateBox.h"
#include "GUIDialog/DialogCreateCylinder.h"
#include "GUIDialog/DialogCreateSphere.h"

namespace Oper
{
	bool OperModelManager::execGUI()
	{
		QDialog* dlg = nullptr;
		if (_emitter->objectName() == "actionCreateBox") {
			dlg = new GUI::CreateBoxDialog(this);
		}
		else if (_emitter->objectName() == "actionCreateCylinder") {
			dlg = new GUI::CreateCylinderDialog(this);
		}
		else if (_emitter->objectName() == "actionCreateSphere") {
			dlg = new GUI::CreateSphereDialog(this);
		}
		if (dlg != nullptr) dlg->show();
		return false;
	}

	bool OperModelManager::execProfession()
	{

		return true;
	}

}
