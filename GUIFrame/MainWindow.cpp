#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ControlPanelWidget.h"
#include "PropertyWidget.h"
#include "RenderWidget.h"
#include "GroupPropertyWidget.h"
#include "MainMenu.h"
#include "ActionEventHandler.h"
#include "MainTreeWidget.h"

#include <SARibbonBar.h>
#include <SARibbonApplicationButton.h>
#include <SARibbonQuickAccessBar.h>
#include <SARibbonTabBar.h>

#include <QString>
#include <QMenu>
#include <QDebug>
#include <QToolBar>
#include <QHash>
#include <QApplication>
#include <QCoreApplication>
#include <QPainter>
#include <QColor>
#include <QSplitter>
#include <QGridLayout>
#include <QStatusBar>
#include <QLabel>


namespace GUI
{
	MainWindow::MainWindow(QWidget *parent) : SARibbonMainWindow(parent)
	{
        _currentWidget = new QWidget(this);

        _ribbonBar = this->ribbonBar();
        
        //使用Office2013风格
        sa_set_ribbon_theme(_ribbonBar, SARibbonTheme::RibbonThemeOffice2013);

        //设置顶部线条颜色
        _ribbonBar->setTabBarBaseLineColor(QColor(186, 201, 219));

        _ribbonBar->setRibbonStyle(SARibbonBar::RibbonStyleLooseThreeRow);
        _ribbonBar->setFont(_font);

        setWindowTitle("FastCAE");
        _ribbonBar->setWindowTitleTextColor(Qt::black);

		init();
	}

	MainWindow::~MainWindow()
	{

	}

	ActionEventHandler * MainWindow::getActionEventHandle() const
	{
		return m_ActionHandler;
	}

	void MainWindow::init()
	{
		m_ActionHandler = new ActionEventHandler;
		initCentralWidget();
		
        initApplicationButton();
        initGeometry();
        initMesh();
        initSetting();
        initResult();
        initHelp();
	}

	void MainWindow::initCentralWidget()
	{
		// 子部件水平排布
		QSplitter *spliterLayout = new QSplitter(Qt::Horizontal);
		spliterLayout->setMouseTracking(true);
		spliterLayout->setHandleWidth(5);

        _treeWidget = new MainTreeWidget(this);
		m_PropertyWidget = new PropertyWidget(this);
		m_RenderWidget = new RenderWidget(this);
		m_GroupPropertyWidget = new GroupPropertyWidget(this);

		spliterLayout->addWidget(_treeWidget);
		spliterLayout->addWidget(m_PropertyWidget);
		spliterLayout->addWidget(m_RenderWidget);
		spliterLayout->addWidget(m_GroupPropertyWidget);
		// 设置大小
		spliterLayout->setSizes({ 200, 300, 1000, 200 });

        auto mainLayout = new QGridLayout();
        mainLayout->setObjectName("CentralGridLayout");
        mainLayout->setContentsMargins(5, 5, 5, 0);
        mainLayout->addWidget(spliterLayout);

        _currentWidget->setLayout(mainLayout);
        setCentralWidget(_currentWidget);
	}

    void MainWindow::initApplicationButton()
    {
        //文件部分添加
        QAbstractButton* fileAppButton = _ribbonBar->applicationButton();
        fileAppButton->setText(tr("File"));
        fileAppButton->setFixedWidth(60);

        QMenu* menu = nullptr;
        QAction* action = nullptr;
    }

    void MainWindow::initHome()
    {
        QString type = tr("Home");
        SARibbonCategory* gategory = _ribbonBar->addCategoryPage(type);
        _ribbonBar->raiseCategory(gategory);

        QAction* action = nullptr;
        //文件部分按钮添加
        SARibbonPannel* pannel = gategory->addPannel(tr("File"));
    }

    void MainWindow::initGeometry()
    {
        QString type = tr("Geometry");
        SARibbonCategory* gategory = _ribbonBar->addCategoryPage(type);
        _ribbonBar->raiseCategory(gategory);

        QAction* action = nullptr;
        SARibbonPannel* pannel = gategory->addPannel(tr("Geometry"));
    }

    void MainWindow::initMesh()
    {
        QString type = tr("Mesh");
        SARibbonCategory* gategory = _ribbonBar->addCategoryPage(type);
        _ribbonBar->raiseCategory(gategory);

        QAction* action = nullptr;
        SARibbonPannel* pannel = gategory->addPannel(tr("Mesh import"));
    }

    void MainWindow::initSetting()
    {
        QString type = tr("Setting");
        SARibbonCategory* gategory = _ribbonBar->addCategoryPage(type);
        _ribbonBar->raiseCategory(gategory);

        QAction* action = nullptr;
        SARibbonPannel* pannel = gategory->addPannel(tr("calculate setting"));
    }

    void MainWindow::initResult()
    {
        QString type = tr("Result");
        SARibbonCategory* gategory = _ribbonBar->addCategoryPage(type);
        _ribbonBar->raiseCategory(gategory);

        QAction* action = nullptr;
        SARibbonPannel* pannel = gategory->addPannel(tr("Result"));
    }

    void MainWindow::initHelp()
    {
        QString type = tr("Help");
        SARibbonCategory* gategory = _ribbonBar->addCategoryPage(type);
        _ribbonBar->raiseCategory(gategory);

        QAction* action = nullptr;
        SARibbonPannel* pannel = gategory->addPannel(tr("Help"));
    }

	RenderWidget * MainWindow::getRenderWidget() const
	{
		return m_RenderWidget;
	}

	PropertyWidget * MainWindow::getPropertyWidget() const
	{
		return m_PropertyWidget;
	}

	GroupPropertyWidget * MainWindow::getGroupPropertyWidget() const
	{
		return m_GroupPropertyWidget;
	}

}
