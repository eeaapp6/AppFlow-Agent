#include "PropertyWidget.h"
#include <QVariant>
#include "MainWindow.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QDebug>

namespace GUI
{
	PropertyWidget::PropertyWidget(MainWindow *mainWindow) : PanelWidgetBase(mainWindow)
	{
		this->setTitle(tr("Property Area"));

		auto area = new QScrollArea(this);
		area->setObjectName("PropertyArea");

		auto areaWidget = new QWidget;
		areaWidget->setObjectName("PropertyAreaWidget");

		area->setWidget(areaWidget);

		auto vlayout = new QVBoxLayout(areaWidget);
		areaWidget->setLayout(vlayout);

		setWidget(area);

	}


}
