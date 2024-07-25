#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ControlPanelWidget.h"
#include "PropertyWidget.h"
#include "RenderWidget.h"
#include "GroupPropertyWidget.h"
#include "MainMenu.h"
#include "ActionEventHandler.h"
#include "MainTreeWidget.h"
#include "TabWidget.h"

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

        QList<QAction*> actionList = this->findChildren<QAction*>();
        for (QAction* action : actionList) {
            if (action == nullptr)continue;
            connect(action, SIGNAL(triggered()), this->getActionEventHandle(), SLOT(execOperator()));
        }

        _ribbonBar->setCurrentIndex(0);
    }

    MainWindow::~MainWindow()
    {

    }

    ActionEventHandler * MainWindow::getActionEventHandle() const
    {
        return m_ActionHandler;
    }

    QAction * MainWindow::createAction(const QString & toolTip, const QString & objectName, const QString & iconPath, const QString & iconText)
    {
        // 实例化一个action
        QAction* tempAction = new QAction(this);

        // 设置action的ObjectName
        tempAction->setObjectName(objectName);
        // 设置action的ToolTip
        tempAction->setToolTip(toolTip);
        // 设置action的图标
        changeAction(tempAction, iconPath, iconText);

        return tempAction;
    }

    void MainWindow::init()
    {
        m_ActionHandler = new ActionEventHandler;
        initCentralWidget();

        initApplicationButton();
        initHome();
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
        _tabWidgete = new TabWidget(this);
        m_GroupPropertyWidget = new GroupPropertyWidget(this);

        //添加树界面
        spliterLayout->addWidget(_treeWidget);
        //添加属性界面
        spliterLayout->addWidget(m_PropertyWidget);

        QSplitter* verLayout = new QSplitter(Qt::Vertical);
        spliterLayout->setMouseTracking(true);
        spliterLayout->setHandleWidth(5);
        verLayout->addWidget(m_RenderWidget);
        verLayout->addWidget(_tabWidgete);

        spliterLayout->addWidget(verLayout);
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

        //导入几何文件
        action = createAction(tr("import geometry"), "actionImportGeometry");
        action->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+G", nullptr));
        fileAppButton->addAction(action);

        //导入网格文件
        action = createAction(tr("import mesh"), "actionImportMesh");
        action->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+M", nullptr));
        fileAppButton->addAction(action);

        //工作目录
        action = createAction(tr("Working Dir"), "actionWorkingDir");
        fileAppButton->addAction(action);
    }

    void MainWindow::initHome()
    {
        QString type = tr("Home");
        SARibbonCategory* gategory = _ribbonBar->addCategoryPage(type);
        _ribbonBar->raiseCategory(gategory);

        QAction* action = nullptr;
        QMenu* menu = nullptr;

        //项目
        SARibbonPannel* pannel = gategory->addPannel(tr("Object"));
        menu = new QMenu(tr("New"), this);
        action = createAction(tr("New"), "actionNew");
        action->setMenu(menu);
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);

        menu = new QMenu(tr("Open"), this);
        action = createAction(tr("Open"), "actionOpen");
        action->setMenu(menu);
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);

        menu = new QMenu(tr("Save"), this);
        action = createAction(tr("Save"), "actionSave");
        action->setMenu(menu);
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);

        //模型结构
        pannel = gategory->addPannel(tr("Model"));
        menu = new QMenu(tr("Geometry import"), this);
        action = createAction(tr("Geometry import"), "actionImportGeometry");
        action->setMenu(menu);
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);

        menu = new QMenu(tr("Mesh import"), this);
        action = createAction(tr("Mesh import"), "actionImportMesh");
        action->setMenu(menu);
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);

        //三维交互
        pannel = gategory->addPannel(tr("View"));
        action = createAction(tr("Auto"), "actionViewPan");
        changeAction(action, ":FITKIcons/icoR_viewPan.svg");
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);

        action = createAction(tr("X forward"), "actionViewLeft");
        changeAction(action, ":FITKIcons/icoR_viewLeft.svg");
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);

        action = createAction(tr("X negative"), "actionViewRight");
        changeAction(action, ":FITKIcons/icoR_viewRight.svg");
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);

        action = createAction(tr("Y forward"), "actionViewTop");
        changeAction(action, ":FITKIcons/icoR_viewTop.svg");
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);

        action = createAction(tr("Y negative"), "actionViewBottom");
        changeAction(action, ":FITKIcons/icoR_viewBottom.svg");
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);

        action = createAction(tr("Z forward"), "actionViewFront");
        changeAction(action, ":FITKIcons/icoR_viewFront.svg");
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);

        action = createAction(tr("Z negative"), "actionViewBack");
        changeAction(action, ":FITKIcons/icoR_viewBack.svg");
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
    }

    void MainWindow::initGeometry()
    {
        QString type = tr("Geometry");
        SARibbonCategory* gategory = _ribbonBar->addCategoryPage(type);
        _ribbonBar->raiseCategory(gategory);

        QAction* action = nullptr;
        SARibbonPannel* pannel = gategory->addPannel(tr("Geometry"));
        action = createAction(tr("Import Geometry"), "actionImportGeometry", "", tr("import geometry"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);

        pannel = gategory->addPannel(tr("3D model"));
        action = createAction(tr("Create Cube"), "actionGeoCubeCreate", "", tr("Create Cube"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
        action = createAction(tr("Create Sphere"), "actionGeoSphereCreate", "", tr("Create Sphere"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
        action = createAction(tr("Create Cone"), "actionGeoConeCreate", "", tr("Create Cone"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
        action = createAction(tr("Create Cirque"), "actionGeoCirqueCreate", "", tr("Create Cirque"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
        action = createAction(tr("Create Cylinder"), "actionGeoCylinderCreate", "", tr("Create Cylinder"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
        action = createAction(tr("Create Spiral"), "actionGeoSpiralCreate", "", tr("Create Spiral"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
    }

    void MainWindow::initMesh()
    {
        QString type = tr("Mesh");
        SARibbonCategory* gategory = _ribbonBar->addCategoryPage(type);
        _ribbonBar->raiseCategory(gategory);


        QAction* action = nullptr;

        // 网格导入
        auto pannel = gategory->addPannel(tr("Import Mesh"));
        auto menu = new QMenu(tr("Import Mesh"), this);
        action = createAction(tr("Import Mesh"), "actionImportMesh");
        action->setIcon(QIcon(":/icons/displayedge.png"));
        action->setMenu(menu);
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);

        action = createAction(tr("Tet"), "actionTetMesh", ":/icons/displayedge.png", tr("Cube"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
        action = createAction(tr("Hex"), "actionHexMesh", ":/icons/displayedge.png", tr("Hex"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
        action = createAction(tr("Tri"), "actionTriMesh", ":/icons/displayedge.png", tr("Tri"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);
        action = createAction(tr("Quad"), "actionQuadMesh", ":/icons/displayedge.png", tr("Quad"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Medium);

        // 网格质量检查
        pannel = gategory->addPannel(tr("Check Mesh"));
        action = createAction(tr("Config A"), "actionConfigAMesh", ":/icons/displayedge.png", tr("Config A"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);
        action = createAction(tr("Config B"), "actionConfigBMesh", ":/icons/displayedge.png", tr("Config B"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);
        action = createAction(tr("Statistics"), "actionMeshStat", ":/icons/displayedge.png", tr("Statistics"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);

        // 网格操作
        pannel = gategory->addPannel(tr("Oper Mesh"));
        action = createAction(tr("Mesh"), "actionMesh", ":/icons/displayedge.png", tr("Mesh"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);
        action = createAction(tr("Extrude"), "action", ":/icons/displayedge.png", tr("Extrude"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);
        action = createAction(tr("Rotate"), "actionHexMesh", ":/icons/displayedge.png", tr("Rotate"));
        pannelAddAction(pannel, action, SARibbonPannelItem::Large);
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

    bool MainWindow::changeAction(QAction * action, const QString iconPath, const QString & iconText)
    {
        if (action == nullptr)return false;
        //图片修改
        if (!iconPath.isEmpty()) {
            action->setIcon(QIcon(iconPath));
        };

        //图注修改
        if (iconText.isEmpty()) {
            action->setText(action->toolTip());
        }
        else {
            action->setText(iconText);
        }

        action->setFont(_font);
        return true;
    }

    void MainWindow::pannelAddAction(SARibbonPannel * pannel, QAction * action, SARibbonPannelItem::RowProportion actionType)
    {
        //pannel中添加action
        if (pannel == nullptr || action == nullptr)return;
        pannel->addAction(action, actionType);
    }

    MainTreeWidget* MainWindow::getTreeWidget() const
    {
        return _treeWidget;
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
