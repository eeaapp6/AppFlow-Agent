#include "OperatorsImportManager.h"

#include <QFileDialog>
#include <QApplication>

namespace ModelOper {
    OperatorsImportManager::OperatorsImportManager()
    {

    }

    OperatorsImportManager::~OperatorsImportManager()
    {
    }

    bool OperatorsImportManager::execGUI()
    {
        QString workDir = QApplication::applicationDirPath();
        QFileDialog fileDialog;
        if (_senderName == "actionImportGeometry") {
            fileDialog.getOpenFileName(_mainWindow, tr("Import Geometry"), workDir, tr("File(*.stp ; *.igs)"));
        }
        else  if (_senderName == "actionImportMesh") {
            fileDialog.getOpenFileName(_mainWindow, tr("Import Mesh"), workDir);
        }
        return true;
    }

    bool OperatorsImportManager::execProfession()
    {
        return true;
    }

}

