#include "OperatorsCFDPost.h"

#include "FITK_Component/FITKWidget/FITKWorkingDirDialog.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"

#include <QFileDialog>

namespace ModelOper
{
    bool OperatorsCFDPost::execGUI()
    {
        QString workindDir = "";
        AppFrame::FITKAppSettings* settings = FITKAPP->getAppSettings();
        if (settings == nullptr) {
            workindDir = settings->getWorkingDir();
        }
        

        QString dirPath = QFileDialog::getExistingDirectory(nullptr, tr("Post Path"), workindDir, QFileDialog::ShowDirsOnly);

        if (dirPath.isEmpty()) {
            return false;
        }

        settings->setValue("CFDPostPath", dirPath);
        settings->write();

        return true;
    }
}

