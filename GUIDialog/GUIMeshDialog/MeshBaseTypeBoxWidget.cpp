#include "MeshBaseTypeBoxWidget.h"
#include "ui_MeshBaseTypeBoxWidget.h"
#include "MeshBaseWidget.h"

#include "OperatorsInterface/GraphEventOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeBox.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeCylinder.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeSphere.h"

#include <QtMath>

namespace GUI
{
    MeshBaseTypeBoxWidget::MeshBaseTypeBoxWidget(QWidget* parent) :
        MeshBaseTypeWidgetBase(parent)
    {
        _ui = new Ui::MeshBaseTypeBoxWidget();
        _ui->setupUi(this);
        //创建预览渲染对象
        _graphObj = new Interface::FITKRegionMeshSizeBox();
        init();

        _ui->pushButton_2->hide();
    }

    MeshBaseTypeBoxWidget::~MeshBaseTypeBoxWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }

        if (_graphObj) {
            delete _graphObj;
            _graphObj = nullptr;
        }

        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->reRender();
    }

    void MeshBaseTypeBoxWidget::init()
    {
        _ui->comboBox_X1->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_X1->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_X1->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_X1->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_X0->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_X0->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_X0->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_X0->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_Y1->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_Y1->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_Y1->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_Y1->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_Y0->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_Y0->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_Y0->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_Y0->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_Z1->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_Z1->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_Z1->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_Z1->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        _ui->comboBox_Z0->addItem(tr("Patch"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTPatch);
        _ui->comboBox_Z0->addItem(tr("Wall"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTWall);
        _ui->comboBox_Z0->addItem(tr("Sym"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTSymmetry);
        _ui->comboBox_Z0->addItem(tr("Empty"), Interface::FITKAbstractRegionMeshSize::BoundaryType::BTEmpty);

        connect(_ui->lineEdit_BasePoint1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_BasePoint2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_BasePoint3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Dimensions1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Dimensions2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Dimensions3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Division1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Division2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Division3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Grading1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Grading2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Grading3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->comboBox_X0, SIGNAL(activated(int)), this, SLOT(slotSaveValue()));
        connect(_ui->comboBox_X1, SIGNAL(activated(int)), this, SLOT(slotSaveValue()));
        connect(_ui->comboBox_Y0, SIGNAL(activated(int)), this, SLOT(slotSaveValue()));
        connect(_ui->comboBox_Y1, SIGNAL(activated(int)), this, SLOT(slotSaveValue()));
        connect(_ui->comboBox_Z0, SIGNAL(activated(int)), this, SLOT(slotSaveValue()));
        connect(_ui->comboBox_Z1, SIGNAL(activated(int)), this, SLOT(slotSaveValue()));
    }

    bool MeshBaseTypeBoxWidget::checkValue()
    {
        return true;
    }

    bool MeshBaseTypeBoxWidget::setDataToWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        Interface::FITKRegionMeshSizeBox* boxObj = dynamic_cast<Interface::FITKRegionMeshSizeBox*>(obj);
        if (boxObj == nullptr)return false;

        this->blockSignals(true);

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

        _ui->comboBox_X0->setCurrentIndex(_ui->comboBox_X0->findData(boxObj->getBoundary(0)));
        _ui->comboBox_X1->setCurrentIndex(_ui->comboBox_X1->findData(boxObj->getBoundary(1)));
        _ui->comboBox_Y0->setCurrentIndex(_ui->comboBox_Y0->findData(boxObj->getBoundary(2)));
        _ui->comboBox_Y1->setCurrentIndex(_ui->comboBox_Y1->findData(boxObj->getBoundary(3)));
        _ui->comboBox_Z0->setCurrentIndex(_ui->comboBox_Z0->findData(boxObj->getBoundary(4)));
        _ui->comboBox_Z1->setCurrentIndex(_ui->comboBox_Z1->findData(boxObj->getBoundary(5)));

        this->blockSignals(false);
        return true;
    }

    bool MeshBaseTypeBoxWidget::getDataFromWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        Interface::FITKRegionMeshSizeBox* boxObj = dynamic_cast<Interface::FITKRegionMeshSizeBox*>(obj);
        if (boxObj == nullptr)return false;

        double basicPoint[3] = { 0,0,0 };
        basicPoint[0] = _ui->lineEdit_BasePoint1->text().toDouble();
        basicPoint[1] = _ui->lineEdit_BasePoint2->text().toDouble();
        basicPoint[2] = _ui->lineEdit_BasePoint3->text().toDouble();
        boxObj->setPoint1(basicPoint);

        double length[3] = { 0,0,0 };
        length[0] = _ui->lineEdit_Dimensions1->text().toDouble();
        length[1] = _ui->lineEdit_Dimensions2->text().toDouble();
        length[2] = _ui->lineEdit_Dimensions3->text().toDouble();
        boxObj->setLength(length);

        boxObj->setDivision(0, _ui->lineEdit_Division1->text().toInt());
        boxObj->setDivision(1, _ui->lineEdit_Division2->text().toInt());
        boxObj->setDivision(2, _ui->lineEdit_Division3->text().toInt());

        boxObj->setGrading(0, _ui->lineEdit_Grading1->text().toDouble());
        boxObj->setGrading(1, _ui->lineEdit_Grading2->text().toDouble());
        boxObj->setGrading(2, _ui->lineEdit_Grading3->text().toDouble());

        boxObj->insertBoundary(0, _ui->comboBox_X0->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>());
        boxObj->insertBoundary(1, _ui->comboBox_X1->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>());
        boxObj->insertBoundary(2, _ui->comboBox_Y0->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>());
        boxObj->insertBoundary(3, _ui->comboBox_Y1->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>());
        boxObj->insertBoundary(4, _ui->comboBox_Z0->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>());
        boxObj->insertBoundary(5, _ui->comboBox_Z1->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>());

        return true;
    }

    void MeshBaseTypeBoxWidget::updateGeometryGraph()
    {
        if (_graphObj == nullptr)return;
        getDataFromWidget(_graphObj);

        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;

        graphOper->updateGraph(_graphObj->getDataObjectID());
        graphOper->reRender(true);
    }

    void MeshBaseTypeBoxWidget::on_pushButton_AutoSize_clicked()
    {
        Interface::FITKGeoCommandList* geoManager = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geoManager == nullptr) return;

        double minPoint[3] = { 9e66,9e66,9e66 };
        double maxPoint[3] = { -9e66, -9e66, -9e66 };
        geoManager->getBoundaryBox(minPoint, maxPoint);

        double xCount = maxPoint[0] - minPoint[0];
        double yCount = maxPoint[1] - minPoint[1];
        double zCount = maxPoint[2] - minPoint[2];
        double rate = qSqrt(xCount*xCount + yCount * yCount + zCount * zCount)*0.005;

        double resultMinPoint[3] = { 0,0,0 };
        resultMinPoint[0] = minPoint[0] - rate;
        resultMinPoint[1] = minPoint[1] - rate;
        resultMinPoint[2] = minPoint[2] - rate;

        double resultMaxPoint[3] = { 0,0,0 };
        resultMaxPoint[0] = maxPoint[0] + rate;
        resultMaxPoint[1] = maxPoint[1] + rate;
        resultMaxPoint[2] = maxPoint[2] + rate;

        double XExtent = resultMaxPoint[0] - resultMinPoint[0];
        double YExtent = resultMaxPoint[1] - resultMinPoint[1];
        double ZExtent = resultMaxPoint[2] - resultMinPoint[2];

        _ui->lineEdit_BasePoint1->setText(QString::number(resultMinPoint[0]));
        _ui->lineEdit_BasePoint2->setText(QString::number(resultMinPoint[1]));
        _ui->lineEdit_BasePoint3->setText(QString::number(resultMinPoint[2]));

        _ui->lineEdit_Dimensions1->setText(QString::number(XExtent));
        _ui->lineEdit_Dimensions2->setText(QString::number(YExtent));
        _ui->lineEdit_Dimensions3->setText(QString::number(ZExtent));

        if (_meshBaseWidget)_meshBaseWidget->saveValue();
        updateGeometryGraph();
    }

    void MeshBaseTypeBoxWidget::slotSaveValue()
    {
        _meshBaseWidget->saveValue();
        updateGeometryGraph();
    }
}


