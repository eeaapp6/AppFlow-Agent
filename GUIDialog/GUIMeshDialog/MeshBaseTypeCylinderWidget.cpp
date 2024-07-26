#include "MeshBaseTypeCylinderWidget.h"
#include "ui_MeshBaseTypeCylinderWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeBox.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeCylinder.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeSphere.h"

#include <QMessageBox>
#include <QtMath>

namespace GUI
{
    MeshBaseTypeCylinderWidget::MeshBaseTypeCylinderWidget()
    {
        _ui = new Ui::MeshBaseTypeCylinderWidget();
        _ui->setupUi(this);

        init();
    }

    MeshBaseTypeCylinderWidget::~MeshBaseTypeCylinderWidget()
    {
        if (_ui)delete _ui;
    }

    void MeshBaseTypeCylinderWidget::init()
    {
        _ui->comboBox_FirstDisk->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_FirstDisk->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_FirstDisk->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_FirstDisk->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_SecondDisk->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_SecondDisk->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_SecondDisk->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_SecondDisk->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_Cylinder->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_Cylinder->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_Cylinder->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_Cylinder->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);
    }

    bool MeshBaseTypeCylinderWidget::checkValue()
    {
        auto outputMessage = [&](QString message) {
            QMessageBox::critical(nullptr, tr("Error Information"), message);
        };

        double axisPoint1 = _ui->lineEdit_AxisPoint1->text().toDouble();
        double axisPoint2 = _ui->lineEdit_AxisPoint2->text().toDouble();
        double axisPoint3 = _ui->lineEdit_AxisPoint3->text().toDouble();
        if (axisPoint1 == 0 && axisPoint2 == 0 && axisPoint3 == 0) {
            outputMessage(tr("AxisX = 0 , AxisY = 0 , AxisZ = 0"));
            return false;
        }
        else
        {
            double mulRatio = qSqrt(axisPoint1*axisPoint1 + axisPoint2 * axisPoint2 + axisPoint3 * axisPoint3);
            _ui->lineEdit_AxisPoint1->setText(QString::number(axisPoint1 / mulRatio));
            _ui->lineEdit_AxisPoint2->setText(QString::number(axisPoint2 / mulRatio));
            _ui->lineEdit_AxisPoint3->setText(QString::number(axisPoint3 / mulRatio));
        }
        return true;
    }

    bool MeshBaseTypeCylinderWidget::setDataToWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        Interface::FITKRegionMeshSizeCylinder* cylinderObj = dynamic_cast<Interface::FITKRegionMeshSizeCylinder*>(obj);
        if (cylinderObj == nullptr)return false;

        double origin[3] = { 0,0,0 };
        cylinderObj->getLocation(origin);
        _ui->lineEdit_OriginPoint1->setText(QString::number(origin[0]));
        _ui->lineEdit_OriginPoint2->setText(QString::number(origin[1]));
        _ui->lineEdit_OriginPoint3->setText(QString::number(origin[2]));

        double axis[3] = { 0,0,0 };
        cylinderObj->getDirection(axis);
        _ui->lineEdit_AxisPoint1->setText(QString::number(axis[0]));
        _ui->lineEdit_AxisPoint2->setText(QString::number(axis[1]));
        _ui->lineEdit_AxisPoint3->setText(QString::number(axis[2]));

        _ui->lineEdit_Length->setText(QString::number(cylinderObj->getLength()));
        _ui->lineEdit_Radius->setText(QString::number(cylinderObj->getRadius()));

        _ui->lineEdit_Division1->setText(QString::number(cylinderObj->getDivision(0)));
        _ui->lineEdit_Division2->setText(QString::number(cylinderObj->getDivision(1)));
        _ui->lineEdit_Division3->setText(QString::number(cylinderObj->getDivision(2)));

        _ui->lineEdit_Grading1->setText(QString::number(cylinderObj->getGrading(0)));
        _ui->lineEdit_Grading2->setText(QString::number(cylinderObj->getGrading(1)));

        _ui->comboBox_FirstDisk->setCurrentIndex(_ui->comboBox_FirstDisk->findData(cylinderObj->getBoundary(0)));
        _ui->comboBox_SecondDisk->setCurrentIndex(_ui->comboBox_SecondDisk->findData(cylinderObj->getBoundary(1)));
        _ui->comboBox_Cylinder->setCurrentIndex(_ui->comboBox_Cylinder->findData(cylinderObj->getBoundary(2)));
        return true;
    }

    bool MeshBaseTypeCylinderWidget::getDataFromWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        Interface::FITKRegionMeshSizeCylinder* cylinderObj = dynamic_cast<Interface::FITKRegionMeshSizeCylinder*>(obj);
        if (cylinderObj == nullptr)return false;

        double origin[3] = { 0,0,0 };
        origin[0] = _ui->lineEdit_OriginPoint1->text().toDouble();
        origin[1] = _ui->lineEdit_OriginPoint2->text().toDouble();
        origin[2] = _ui->lineEdit_OriginPoint3->text().toDouble();
        cylinderObj->setLocation(origin);

        double axis[3] = { 0,0,0 };
        axis[0] = _ui->lineEdit_AxisPoint1->text().toDouble();
        axis[1] = _ui->lineEdit_AxisPoint2->text().toDouble();
        axis[2] = _ui->lineEdit_AxisPoint3->text().toDouble();
        cylinderObj->setDirection(axis);

        cylinderObj->setLength(_ui->lineEdit_Length->text().toDouble());
        cylinderObj->setRadius(_ui->lineEdit_Radius->text().toDouble());

        cylinderObj->setDivision(0, _ui->lineEdit_Division1->text().toInt());
        cylinderObj->setDivision(1, _ui->lineEdit_Division2->text().toInt());
        cylinderObj->setDivision(2, _ui->lineEdit_Division3->text().toInt());

        cylinderObj->setGrading(0, _ui->lineEdit_Grading1->text().toDouble());
        cylinderObj->setGrading(1, _ui->lineEdit_Grading2->text().toDouble());

        cylinderObj->insertBoundary(0, _ui->comboBox_FirstDisk->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>());
        cylinderObj->insertBoundary(1, _ui->comboBox_SecondDisk->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>());
        cylinderObj->insertBoundary(2, _ui->comboBox_Cylinder->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>());
        return true;
    }
}
