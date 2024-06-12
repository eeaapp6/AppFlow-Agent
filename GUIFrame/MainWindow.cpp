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


		MenuActionItem spearator;
		// 文件菜单
		MenuActionItem fileOpen(tr("Open"), "actionFileOpen", QIcon(":/icons/open.png"));
		MenuActionItem fileSave(tr("Save"), "actionFileSave", QIcon(":/icons/save.png"));
		MenuActionItem fileSaveAs(tr("SaveAs"), "actionFileSaveAs", QIcon(":/icons/saveas.png"));
		MenuActionItem fileExit(tr("Exit"), "actionFileExit");

		// 视图菜单
		MenuActionItem viewAutoFit(tr("Auto Fit"), "actionViewAutoFit", QIcon(":/icons/autofit.png"));
		MenuActionItem viewFront(tr("Front"), "actionViewFront", QIcon(":/icons/view_front.png"));
		MenuActionItem viewBack(tr("Back"), "actionViewBack", QIcon(":/icons/view_back.png"));
		MenuActionItem viewTop(tr("Top"), "actionViewTop", QIcon(":/icons/view_top.png"));
		MenuActionItem viewBottom(tr("Bottom"), "actionViewBottom", QIcon(":/icons/view_bottom.png"));
		MenuActionItem viewLeft(tr("Left"), "actionViewLeft", QIcon(":/icons/view_left.png"));
		MenuActionItem viewRight(tr("Right"), "actionViewRight", QIcon(":/icons/view_right.png"));
		MenuActionItem viewDisplayNode(tr("Display Node"), "actionViewDisplayNode", true, "DisplayMode");
		MenuActionItem viewDisplayWireFrame(tr("Display WireFrame"), "actionViewDisplayWireFrame", true, "DisplayMode");
		MenuActionItem viewDisplaySurface(tr("Display Surface"), "actionViewDisplaySurface", true, "DisplayMode");
		MenuActionItem viewDisplay(tr("Display"), "actionViewDisplay", { &viewDisplayNode, &viewDisplayWireFrame, &viewDisplaySurface });

		// 创建几何
		MenuActionItem createBox(tr("Box"), "actionCreateBox", QIcon(":/icons/createbox.png"));
		MenuActionItem createCylinder(tr("Cylinder"), "actionCreateCylinder", QIcon(":/icons/createcylinder.png"));
		MenuActionItem createSphere(tr("Sphere"), "actionCreateSphere", QIcon(":/icons/createsphere.png"));
		// 菜单栏
		m_MainMenu->addMenu(tr("File"), { &fileOpen, &spearator, &fileSave, &fileSaveAs, &spearator, &fileExit });
		m_MainMenu->addMenu(tr("View"), { &viewAutoFit, &spearator, &viewFront, &viewBack, &viewTop, &viewBottom, &viewLeft, &viewRight, &spearator, &viewDisplay });
		m_MainMenu->addMenu(tr("Create"), { &createBox, &createCylinder, &createSphere });
		// 工具栏菜单
		m_MainMenu->addToolMenu(tr("File"), { &fileOpen, &fileSave, &fileSaveAs });
		m_MainMenu->addToolMenu(tr("View"), { &viewAutoFit, &viewFront, &viewBack, &viewTop, &viewBottom, &viewLeft, &viewRight });
		m_MainMenu->addToolMenu(tr("Create"), { &createBox, &createCylinder, &createSphere });

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

	MainMenuBase * MainWindow::getMainMenuBase() const
	{
		return m_MainMenu;
	}

	ControlPanelWidget * MainWindow::getControlPanelWidget() const
	{
		return m_ControlWidget;
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
