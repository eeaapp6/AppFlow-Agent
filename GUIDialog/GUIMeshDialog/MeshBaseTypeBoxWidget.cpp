#include "MeshBaseTypeBoxWidget.h"
#include "ui_MeshBaseTypeBoxWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeBox.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeCylinder.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeSphere.h"

namespace GUI
{
    MeshBaseTypeBoxWidget::MeshBaseTypeBoxWidget()
    {
        _ui = new Ui::MeshBaseTypeBoxWidget();
        _ui->setupUi(this);

        init();
    }

    MeshBaseTypeBoxWidget::~MeshBaseTypeBoxWidget()
    {
        if (_ui)delete _ui;
    }

    void MeshBaseTypeBoxWidget::init()
    {
        _ui->lineEdit_BasePoint1->setText("0");
        _ui->lineEdit_BasePoint2->setText("0");
        _ui->lineEdit_BasePoint3->setText("0");

        _ui->lineEdit_Dimensions1->setText("10");
        _ui->lineEdit_Dimensions2->setText("10");
        _ui->lineEdit_Dimensions3->setText("10");

        _ui->lineEdit_Division1->setText("10");
        _ui->lineEdit_Division2->setText("10");
        _ui->lineEdit_Division3->setText("10");

        _ui->lineEdit_Grading1->setText("1.0");
        _ui->lineEdit_Grading2->setText("1.0");
        _ui->lineEdit_Grading3->setText("1.0");

        _ui->comboBox_X0->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_X0->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_X0->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_X0->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_X1->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_X1->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_X1->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_X1->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_Y0->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_Y0->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_Y0->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_Y0->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_Y1->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_Y1->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_Y1->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_Y1->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_Z0->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_Z0->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_Z0->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_Z0->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_Z1->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_Z1->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_Z1->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_Z1->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);
    }

    bool MeshBaseTypeBoxWidget::checkValue()
    {
        return true;
    }

    bool MeshBaseTypeBoxWidget::setDataToWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        Interface::FITKRegionMeshSizeBox* boxObj = dynamic_cast<Interface::FITKRegionMeshSizeBox*>(obj);
        if (boxObj == nullptr)return false;

        double basicPoint[3] = { 0,0,0 };
        boxObj->getPoint1(basicPoint);
        _ui->lineEdit_BasePoint1->setText(QString::number(basicPoint[0]));
        _ui->lineEdit_BasePoint2->setText(QString::number(basicPoint[1]));
        _ui->lineEdit_BasePoint3->setText(QString::number(basicPoint[2]));

        double length[3] = { 0,0,0 };
        boxObj->getLength(length);
        _ui->lineEdit_Dimensions1->setText(QString::number(length[0]));
        _ui->lineEdit_Dimensions2->setText(QString::number(length[1]));
        _ui->lineEdit_Dimensions3->setText(QString::number(length[2]));
        
        
        _ui->lineEdit_Division1->setText(QString::number(boxObj->getDivision(0)));
        _ui->lineEdit_Division2->setText(QString::number(boxObj->getDivision(1)));
        _ui->lineEdit_Division3->setText(QString::number(boxObj->getDivision(2)));

        _ui->lineEdit_Grading1->setText(QString::number(boxObj->getGrading(0)));
        _ui->lineEdit_Grading2->setText(QString::number(boxObj->getGrading(1)));
        _ui->lineEdit_Grading3->setText(QString::number(boxObj->getGrading(2)));

        return true;
    }

    bool MeshBaseTypeBoxWidget::getDataFromWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        Interface::FITKRegionMeshSizeBox* boxObj = dynamic_cast<Interface::FITKRegionMeshSizeBox*>(obj);
        if (boxObj == nullptr)return false;

        double basicPoint[3] = { 0,0,0 };
        basicPoint[0]= _ui->lineEdit_BasePoint1->text().toDouble();
        basicPoint[1]= _ui->lineEdit_BasePoint2->text().toDouble();
        basicPoint[2]= _ui->lineEdit_BasePoint3->text().toDouble();
        boxObj->setPoint1(basicPoint);

        double length[3] = { 0,0,0 };
        length[0] = _ui->lineEdit_Dimensions1->text().toDouble();
        length[1] = _ui->lineEdit_Dimensions2->text().toDouble();
        length[2] = _ui->lineEdit_Dimensions3->text().toDouble();
        boxObj->setLength(length);

        boxObj->setDivision(0, _ui->lineEdit_Division1->text().toInt());
        boxObj->setDivision(1, _ui->lineEdit_Division2->text().toInt());
        boxObj->setDivision(2, _ui->lineEdit_Division3->text().toInt());

        boxObj->setGrading(0, _ui->lineEdit_Grading1->text().toInt());
        boxObj->setGrading(1, _ui->lineEdit_Grading2->text().toInt());
        boxObj->setGrading(2, _ui->lineEdit_Grading3->text().toInt());

        return true;
    }
}
