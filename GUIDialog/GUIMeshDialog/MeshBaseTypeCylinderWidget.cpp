#include "MeshBaseTypeCylinderWidget.h"
#include "ui_MeshBaseTypeCylinderWidget.h"
#include "MeshBaseWidget.h"
#include "CompBaseBoundary.h"

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

#include <QMessageBox>
#include <QtMath>
#include <QToolButton>

namespace GUI
{
    MeshBaseTypeCylinderWidget::MeshBaseTypeCylinderWidget(QWidget* parent) :
        MeshBaseTypeWidgetBase(parent)
    {
        _ui = new Ui::MeshBaseTypeCylinderWidget();
        _ui->setupUi(this);
        _graphObj = new Interface::FITKRegionMeshSizeCylinder();
        init();

        _ui->pushButton_2->hide();
    }

    MeshBaseTypeCylinderWidget::~MeshBaseTypeCylinderWidget()
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

        connect(_ui->lineEdit_OriginPoint1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_OriginPoint2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_OriginPoint3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_AxisPoint1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_AxisPoint2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_AxisPoint3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Length, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Radius, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_RadFraction, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Division1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Division2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Division3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Grading1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Grading2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->label_FirstDisk, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->label_SecondDisk, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->label_Cylinder, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        _ui->label_FirstDisk->setPos(0);
        _ui->label_SecondDisk->setPos(1);
        _ui->label_Cylinder->setPos(2);
        connect(_ui->comboBox_FirstDisk, SIGNAL(activated(int)), this, SLOT(slotSaveValue()));
        connect(_ui->comboBox_SecondDisk, SIGNAL(activated(int)), this, SLOT(slotSaveValue()));
        connect(_ui->comboBox_Cylinder, SIGNAL(activated(int)), this, SLOT(slotSaveValue()));
        connect(_ui->comboBox_FirstDisk, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->comboBox_SecondDisk, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->comboBox_Cylinder, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        _ui->comboBox_FirstDisk->setPos(0);
        _ui->comboBox_SecondDisk->setPos(1);
        _ui->comboBox_Cylinder->setPos(2);
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

        this->blockSignals(true);

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
        _ui->lineEdit_RadFraction->setText(QString::number(cylinderObj->getRadialFraction()));

        _ui->lineEdit_Division1->setText(QString::number(cylinderObj->getDivision(0)));
        _ui->lineEdit_Division2->setText(QString::number(cylinderObj->getDivision(1)));
        _ui->lineEdit_Division3->setText(QString::number(cylinderObj->getDivision(2)));

        _ui->lineEdit_Grading1->setText(QString::number(cylinderObj->getGrading(0)));
        _ui->lineEdit_Grading2->setText(QString::number(cylinderObj->getGrading(1)));

        _ui->comboBox_FirstDisk->setCurrentIndex(_ui->comboBox_FirstDisk->findData(cylinderObj->getBoundary(0)));
        _ui->comboBox_SecondDisk->setCurrentIndex(_ui->comboBox_SecondDisk->findData(cylinderObj->getBoundary(1)));
        _ui->comboBox_Cylinder->setCurrentIndex(_ui->comboBox_Cylinder->findData(cylinderObj->getBoundary(2)));

        this->blockSignals(false);
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
        cylinderObj->setRadialFraction(_ui->lineEdit_RadFraction->text().toDouble());

        cylinderObj->setDivision(0, _ui->lineEdit_Division1->text().toInt());
        cylinderObj->setDivision(1, _ui->lineEdit_Division2->text().toInt());
        cylinderObj->setDivision(2, _ui->lineEdit_Division3->text().toInt());

        cylinderObj->setGrading(0, _ui->lineEdit_Grading1->text().toDouble());
        cylinderObj->setGrading(1, _ui->lineEdit_Grading2->text().toDouble());

        auto type = _ui->comboBox_FirstDisk->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>();
        cylinderObj->insertBoundary(0, type);
        type = _ui->comboBox_SecondDisk->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>();
        cylinderObj->insertBoundary(1, type);
        type = _ui->comboBox_Cylinder->currentData().value<Interface::FITKAbstractRegionMeshSize::BoundaryType>();
        cylinderObj->insertBoundary(2, type);
        return true;
    }

    void MeshBaseTypeCylinderWidget::updateGeometryGraph()
    {
        if (_graphObj == nullptr)return;
        getDataFromWidget(_graphObj);

        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;

        graphOper->updateGraph(_graphObj->getDataObjectID());
        graphOper->reRender(true);
    }

    void MeshBaseTypeCylinderWidget::on_pushButton_AutoSize_clicked()
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

        double origin[3] = { 0,0,0 };
        origin[0] = resultMinPoint[0];
        origin[1] = resultMinPoint[1] + YExtent / 2;
        origin[2] = resultMinPoint[2] + ZExtent / 2;

        double length = resultMaxPoint[0] - resultMinPoint[0];
        double redius = qSqrt(YExtent*YExtent + ZExtent * ZExtent) / 2;

        _ui->lineEdit_OriginPoint1->setText(QString::number(origin[0]));
        _ui->lineEdit_OriginPoint2->setText(QString::number(origin[1]));
        _ui->lineEdit_OriginPoint3->setText(QString::number(origin[2]));
        _ui->lineEdit_AxisPoint1->setText("1");
        _ui->lineEdit_AxisPoint2->setText("0");
        _ui->lineEdit_AxisPoint3->setText("0");
        _ui->lineEdit_Length->setText(QString::number(length));
        _ui->lineEdit_Radius->setText(QString::number(redius));

        if (_meshBaseWidget)_meshBaseWidget->saveValue();
        updateGeometryGraph();
    }

    void MeshBaseTypeCylinderWidget::slotSaveValue()
    {
        _meshBaseWidget->saveValue();
        updateGeometryGraph();
    }

    void MeshBaseTypeCylinderWidget::slotMouseMove()
    {
        if (_graphObj == nullptr)return;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;

        int rowIndex = -1;
        CompBaseBoundaryLabel* label = dynamic_cast<CompBaseBoundaryLabel*>(sender());
        CompBaseBoundaryComboBox* comBox = dynamic_cast<CompBaseBoundaryComboBox*>(sender());
        if (label) {
            rowIndex = label->getPos();
            //label->setStyleSheet("background-color: #ADD8E6;");
        }
        else if (comBox) {
            rowIndex = comBox->getPos();
            //comBox->setStyleSheet("background-color: #ADD8E6;");
        }

        //清除高亮
        graphOper->clearHighlight();

        getDataFromWidget(_graphObj);
        QVector<int> ids = {};
        ids.append(rowIndex);
        graphOper->advHighlight(_graphObj->getDataObjectID(), ids);
        graphOper->reRender(true);

    }
}
