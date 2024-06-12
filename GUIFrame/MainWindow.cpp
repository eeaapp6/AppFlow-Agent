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

#include "ActionEventHandler.h"

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

	ActionEventHandler * MainWindow::getActionEventHandle() const
	{
		return m_ActionHandler;
	}

	void MainWindow::init()
	{
		m_ActionHandler = new ActionEventHandler;
		initMenu();
		initCentralWidget();
		initStatusBar();
	}

	void MainWindow::initMenu()
	{
		// 根据不同风格创建不同的菜单
		m_MainMenu = new MainMenu(this);


		auto spearator = new MenuActionItem;
		// 文件菜单
		auto fileOpen = new MenuActionItem(tr("Open"), "actionFileOpen", QIcon(":/icons/open.png"));
		auto fileSave = new MenuActionItem(tr("Save"), "actionFileSave", QIcon(":/icons/save.png"));
		auto fileSaveAs = new MenuActionItem(tr("SaveAs"), "actionFileSaveAs", QIcon(":/icons/saveas.png"));
		auto fileExit = new MenuActionItem(tr("Exit"), "actionFileExit");

		// 视图菜单
		auto viewAutoFit = new MenuActionItem(tr("Auto Fit"), "actionViewAutoFit", QIcon(":/icons/autofit.png"));
		auto viewFront = new MenuActionItem(tr("Front"), "actionViewFront", QIcon(":/icons/view_front.png"));
		auto viewBack = new MenuActionItem(tr("Back"), "actionViewBack", QIcon(":/icons/view_back.png"));
		auto viewTop = new MenuActionItem(tr("Top"), "actionViewTop", QIcon(":/icons/view_top.png"));
		auto viewBottom = new MenuActionItem(tr("Bottom"), "actionViewBottom", QIcon(":/icons/view_bottom.png"));
		auto viewLeft = new MenuActionItem(tr("Left"), "actionViewLeft", QIcon(":/icons/view_left.png"));
		auto viewRight = new MenuActionItem(tr("Right"), "actionViewRight", QIcon(":/icons/view_right.png"));
		auto viewDisplayNode = new MenuActionItem(tr("Display Node"), "actionViewDisplayNode", true, "DisplayMode");
		auto viewDisplayWireFrame = new MenuActionItem(tr("Display WireFrame"), "actionViewDisplayWireFrame", true, "DisplayMode");
		auto viewDisplaySurface = new MenuActionItem(tr("Display Surface"), "actionViewDisplaySurface", true, "DisplayMode");
		auto viewDisplay = new MenuActionItem(tr("Display"), "actionViewDisplay", { viewDisplayNode, viewDisplayWireFrame, viewDisplaySurface });

		// 创建几何
		auto createBox = new MenuActionItem(tr("Box"), "actionCreateBox", QIcon(":/icons/createbox.png"));
		auto createCylinder = new MenuActionItem(tr("Cylinder"), "actionCreateCylinder", QIcon(":/icons/createcylinder.png"));
		auto createSphere = new MenuActionItem(tr("Sphere"), "actionCreateSphere", QIcon(":/icons/createsphere.png"));
		// 菜单栏
		m_MainMenu->addMenu(tr("File"), { fileOpen, spearator, fileSave, fileSaveAs, spearator, fileExit });
		m_MainMenu->addMenu(tr("View"), { viewAutoFit, spearator, viewFront, viewBack, viewTop, viewBottom, viewLeft, viewRight, spearator, viewDisplay });
		m_MainMenu->addMenu(tr("Create"), { createBox, createCylinder, createSphere });
		// 工具栏菜单
		m_MainMenu->addToolMenu(tr("File"), { fileOpen, fileSave, fileSaveAs });
		m_MainMenu->addToolMenu(tr("View"), { viewAutoFit, viewFront, viewBack, viewTop, viewBottom, viewLeft, viewRight });
		m_MainMenu->addToolMenu(tr("Create"), { createBox, createCylinder, createSphere });

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
