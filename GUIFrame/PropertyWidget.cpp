#include "PropertyWidget.h"
#include <QVariant>
#include "MainWindow.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QDebug>
#include "GUIWidget/PropertyEmptyWidget.h"

namespace GUI
{
	PropertyWidget::PropertyWidget(MainWindow *mainWindow) : PanelWidgetBase(mainWindow)
	{
		this->setTitle(tr("Property Area"));

		auto scrollArea = new QScrollArea(this);
		scrollArea->setWidget(new PropertyEmptyWidget(m_MainWindow, tr("Welcome to FlowApp.")));
		setWidget(scrollArea);

	}


}
