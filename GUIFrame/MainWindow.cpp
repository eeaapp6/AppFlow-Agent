#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ControlPanelWidget.h"
#include "PropertyWidget.h"
#include "RenderWidget.h"
#include "GroupPropertyWidget.h"
#include "MainMenu.h"
#include <QSplitter>
#include <QGridLayout>
#include <QStatusBar>
#include <QLabel>

namespace GUI
{


	MainWindow::MainWindow(QWidget *parent) :
		QMainWindow(parent),
		m_Ui(new Ui::MainWindow)
	{
		m_Ui->setupUi(this);
		setWindowTitle(tr("Flow App"));
		init();
	}

	MainWindow::~MainWindow()
	{
		delete m_Ui;
	}

	void MainWindow::init()
	{
		initMenu();
		initCentralWidget();
		initStatusBar();
	}

	void MainWindow::initMenu()
	{
		// 根据不同风格创建不同的菜单
		m_MainMenu = new MainMenu(this);

		// 文件菜单
		auto fileMenuActions = {
			new MenuActionItem(tr("Open"), "actionFileOpen"),
			new MenuActionItem,
			new MenuActionItem(tr("Save"), "actionFileSave"),
			new MenuActionItem(tr("SaveAs"), "actionFileSaveAs"),
			new MenuActionItem,
			new MenuActionItem(tr("Exit"), "actionFileExit"),
		};
		m_MainMenu->addMenu(tr("File"), fileMenuActions);

		// 视图菜单
		auto viewMenuActions = {
			new MenuActionItem(tr("Auto Fit"), "actionViewAutoFit"),
			new MenuActionItem,
			new MenuActionItem(tr("Front"), "actionViewFront"),
			new MenuActionItem(tr("Back"), "actionViewBack"),
			new MenuActionItem(tr("Top"), "actionViewTop"),
			new MenuActionItem(tr("Bottom"), "actionViewBottom"),
			new MenuActionItem(tr("Left"), "actionViewLeft"),
			new MenuActionItem(tr("Right"), "actionViewRight"),
			new MenuActionItem,
			new MenuActionItem(tr("Display"), "actionViewDisplay",
				{
					new MenuActionItem(tr("Display Node"), "actionViewDisplayNode", true, "DisplayMode"),
					new MenuActionItem(tr("Display WireFrame"), "actionViewDisplayWireFrame", true, "DisplayMode"),
					new MenuActionItem(tr("Display Surface"), "actionViewDisplaySurface", true, "DisplayMode"),
				}
			),
		};
		m_MainMenu->addMenu(tr("View"), viewMenuActions);


		auto fileToolBar = {
			new MenuActionItem(tr("Open"), "actionFileOpen", QIcon(":/icons/open.png")),
			new MenuActionItem(tr("Save"), "actionFileSave", QIcon(":/icons/save.png")),
			new MenuActionItem(tr("SaveAs"), "actionFileSaveAs", QIcon(":/icons/saveas.png")),
		};
		m_MainMenu->addToolMenu(tr("File"), fileToolBar);

	}

	void MainWindow::initCentralWidget()
	{
		// 子部件水平排布
		QSplitter *spliterLayout = new QSplitter(Qt::Horizontal);
		spliterLayout->setMouseTracking(true);
		spliterLayout->setHandleWidth(5);

		m_ControlWidget = new ControlPanelWidget(this);
		m_PropertyWidget = new PropertyWidget(this);
		m_RenderWidget = new RenderWidget(this);
		m_GroupPropertyWidget = new GroupPropertyWidget(this);

		spliterLayout->addWidget(m_ControlWidget);
		spliterLayout->addWidget(m_PropertyWidget);
		spliterLayout->addWidget(m_RenderWidget);
		spliterLayout->addWidget(m_GroupPropertyWidget);
		// 设置大小
		spliterLayout->setSizes({ 200, 300, 1000, 200 });

		auto mainLayout = new QGridLayout();
		mainLayout->setObjectName("CentralGridLayout");
		mainLayout->setContentsMargins(5, 5, 5, 0);
		m_Ui->centralwidget->setLayout(mainLayout);
		mainLayout->addWidget(spliterLayout);
	}

	void MainWindow::initStatusBar()
	{
		auto statusBar = new QStatusBar;
		setStatusBar(statusBar);

		statusBar->addWidget(new QLabel(tr("Welcome to FlowApp")), 1);
	}

}
