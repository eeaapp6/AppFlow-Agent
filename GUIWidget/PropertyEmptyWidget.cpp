#include "PropertyEmptyWidget.h"
#include "ui_PropertyEmptyWidget.h"

namespace GUI
{
	// 构造函数
	PropertyEmptyWidget::PropertyEmptyWidget(MainWindow* mainWindow, QString tip)
		: PropertyChildWidgetBase(mainWindow), m_Ui(new Ui::PropertyEmptyWidget)
	{
		m_Ui->setupUi(this);
		setTip(tip);
	}
	// 析构函数
	PropertyEmptyWidget::~PropertyEmptyWidget()
	{
		delete m_Ui;
	}
	void PropertyEmptyWidget::setTip(QString info)
	{
		m_Ui->lb_tip->setText(info);
	}
}
