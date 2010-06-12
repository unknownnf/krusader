/***************************************************************************
                               krusaderview.cpp
                            -------------------
   copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
   e-mail               : krusader@users.sourceforge.net
   web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
 Description
***************************************************************************

 A

    db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
    88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
    88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
    88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
    88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
    YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                    S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "krusaderview.h"

#include <QGridLayout>
#include <QList>
#include <QKeyEvent>
#include <QEvent>
#include <QtGui/QClipboard>

#include <kstatusbar.h>
#include <kmenubar.h>
#include <kshortcut.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
#include <klibloader.h>

#include "krusader.h"
#include "kractions.h"
#include "krslots.h"
#include "defaults.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "GUI/kcmdline.h"
#include "GUI/kfnkeys.h"
#include "GUI/terminaldock.h"
#include "resources.h"
#include "panelmanager.h"
#include "GUI/profilemanager.h"
#include "Dialogs/percentalsplitter.h"
#include "krservices.h"
#include "Panel/krviewfactory.h"

KrusaderView::KrusaderView(QWidget *parent) : QWidget(parent) {}

void KrusaderView::start(KConfigGroup &cfg, bool restoreSettings, QStringList leftTabs, QStringList rightTabs)
{
    ////////////////////////////////
    // make a 1x1 mainLayout, it will auto-expand:
    mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    // vertical splitter
    vert_splitter = new QSplitter(this);   // splits between panels and terminal/cmdline
    vert_splitter->setOrientation(Qt::Vertical);
    // horizontal splitter
    horiz_splitter = new PercentalSplitter(vert_splitter);
    (terminal_dock = new TerminalDock(vert_splitter)) ->hide();     // create it hidden

    // create a command line thing
    cmdLine = new KCMDLine(this);

    // add a panel manager for each side of the splitter
    leftMng  = new PanelManager(horiz_splitter, true);
    rightMng = new PanelManager(horiz_splitter, false);

    // now, create the panels inside the manager
    KConfigGroup gl(krConfig, "Look&Feel");
    int defaultType = gl.readEntry("Default Panel Type", KrViewFactory::defaultViewId());
    left = leftMng->createPanel(defaultType);
    right = rightMng->createPanel(defaultType);

    // create the function keys widget
    fnKeys = new KFnKeys(this);
    fnKeys->hide();
    fnKeys->setWhatsThis(i18n("Function keys allow performing fast "
                              "operations on files."));

    // and insert the whole thing into the main layout... at last
    mainLayout->addWidget(vert_splitter, 0, 0);    //<>
    mainLayout->addWidget(cmdLine, 1, 0);
    mainLayout->addWidget(fnKeys, 2, 0);
    mainLayout->activate();

    // get the last saved sizes of the splitter
    KConfigGroup group(krConfig, "Private");
    QList<int> lst = group.readEntry("Splitter Sizes", QList<int>());
    if (lst.count() == 0) {
        lst.push_back(100);
        lst.push_back(100);
    } else if (lst.count() == 2 && lst[ 0 ] == 0 && lst[ 1 ] == 0) {
        lst[ 0 ] = 100;
        lst[ 1 ] = 100;
    }

    horiz_splitter->setSizes(lst);

    verticalSplitterSizes = group.readEntry("Terminal Emulator Splitter Sizes", QList<int> ());
    if (verticalSplitterSizes.count() == 2 && verticalSplitterSizes[ 0 ] == 0 && verticalSplitterSizes[ 1 ] == 0)
        verticalSplitterSizes.clear();

    show();

    KUrl leftUrl(QDir::homePath()), rightUrl(QDir::homePath());
    if(leftTabs.count())
        leftUrl = leftTabs[0];
    if(rightTabs.count())
        rightUrl = rightTabs[0];
    rightMng->startPanel(right, rightUrl);
    leftMng->startPanel(left, leftUrl);

    // make the left panel focused at program start
    KrGlobal::activePanel = left;
    ACTIVE_PANEL->gui->slotFocusOnMe();  // left starts out active

    for (int i = 1; i < leftTabs.count(); i++)
        leftMng->slotNewTab(leftTabs[ i ], false);

    for (int j = 1; j < rightTabs.count(); j++)
        rightMng->slotNewTab(rightTabs[ j ], false);

    // this is needed so that all tab labels get updated
    leftMng->layoutTabs();
    rightMng->layoutTabs();

    if(restoreSettings) {
        int         leftActiveTab = cfg.readEntry("Left Active Tab", 0);
        int         rightActiveTab = cfg.readEntry("Right Active Tab", 0);
        bool        leftActive = cfg.readEntry("Left Side Is Active", false);

        if(!leftTabs.count()) {
            leftMng->loadSettings(&cfg, "Left Tab Bar");
            leftMng->setActiveTab(leftActiveTab);
        }
        if(!rightTabs.count()) {
            rightMng->loadSettings(&cfg, "Right Tab Bar");
            rightMng->setActiveTab(rightActiveTab);
        }
        if (leftActive)
            left->slotFocusOnMe();
        else
            right->slotFocusOnMe();
    }
}

// updates the command line whenever current panel or its path changes
//////////////////////////////////////////////////////////
void KrusaderView::slotPathChanged(ListPanel *p)
{
    if(p == ACTIVE_PANEL) {
        cmdLine->setCurrent(p->realPath());
        KConfigGroup cfg = krConfig->group("General");
        if (cfg.readEntry("Send CDs", _SendCDs)) { // hopefully, this is cached in kconfig
            terminal_dock->sendCd(p->realPath());
        }
    }
}

void KrusaderView::cmdLineFocus()    // command line receive's keyboard focus
{
    cmdLine->setFocus();
}

void KrusaderView::cmdLineUnFocus()   // return focus to the active panel
{
    ACTIVE_PANEL->gui->slotFocusOnMe();
}

// Tab - switch focus
void KrusaderView::panelSwitch()
{
    ACTIVE_PANEL->otherPanel()->gui->slotFocusOnMe();
}
void KrusaderView::slotSetActivePanel(ListPanel *p)
{
    KrGlobal::activePanel = p;
    slotPathChanged(p);
}

void KrusaderView::slotTerminalEmulator(bool show)
{
    KConfigGroup cfg = krConfig->group("Look&Feel");
    bool fullscreen = cfg.readEntry("Fullscreen Terminal Emulator", false);
    static bool fnKeysShown = true; // first time init. should be overridden
    static bool cmdLineShown = true;
    static bool statusBarShown = true;
    static bool toolBarShown = true;
    static bool menuBarShown = true;

    if (!show) {    // hiding the terminal
        ACTIVE_PANEL->gui->slotFocusOnMe();
        if (terminal_dock->isTerminalVisible() && !fullscreen)
            verticalSplitterSizes = vert_splitter->sizes();

        terminal_dock->hide();
        QList<int> newSizes;
        newSizes.push_back(vert_splitter->height());
        newSizes.push_back(0);
        vert_splitter->setSizes(newSizes);
        // in full screen, we unhide everything that was hidden before
        if (fullscreen) {
            leftMng->show();
            rightMng->show();
            if (fnKeysShown) fnKeys->show();
            if (cmdLineShown) cmdLine->show();
            if (statusBarShown) krApp->statusBar()->show();
            if (toolBarShown) {
                krApp->toolBar()->show();
                krApp->toolBar("actionsToolBar")->show();
            }
            if (menuBarShown) krApp->menuBar()->show();
        }
        return ;
    }
    // else implied
    terminal_dock->initialise();
    if (terminal_dock->isInitialised()) {        // if we succeeded in creating the konsole
        if (!verticalSplitterSizes.empty())
            vert_splitter->setSizes(verticalSplitterSizes);

        terminal_dock->show();
        slotPathChanged(ACTIVE_PANEL->gui);

        terminal_dock->setFocus();

        krToggleTerminal->setChecked(true);
        // in full screen mode, we hide everything else, but first, see what was actually shown
        if (fullscreen) {
            fnKeysShown = !fnKeys->isHidden();
            cmdLineShown = !cmdLine->isHidden();
            statusBarShown = !krApp->statusBar()->isHidden();
            toolBarShown = !krApp->toolBar()->isHidden();
            menuBarShown = !krApp->menuBar()->isHidden();
            leftMng->hide();
            rightMng->hide();
            fnKeys->hide();
            cmdLine->hide();
            krApp->statusBar()->hide();
            krApp->toolBar()->hide();
            krApp->toolBar("actionsToolBar")->hide();
            krApp->menuBar()->hide();
        }
    } else {
        terminal_dock->hide();
        krToggleTerminal->setChecked(false);
    }
}

void KrusaderView::focusTerminalEmulator()
{
    if (terminal_dock->isTerminalVisible())
        terminal_dock->setFocus();
}

void KrusaderView::switchFullScreenTE()
{
    if (terminal_dock->isTerminalVisible()) {
        KConfigGroup grp = krConfig->group("Look&Feel");
        bool fullscreen = grp.readEntry("Fullscreen Terminal Emulator", false);
        slotTerminalEmulator(false);
        grp.writeEntry("Fullscreen Terminal Emulator", !fullscreen);
        slotTerminalEmulator(true);
    }
}

QList<int> KrusaderView::getTerminalEmulatorSplitterSizes()
{
    if (terminal_dock->isVisible())
        return vert_splitter->sizes();
    else
        return verticalSplitterSizes;
}

void KrusaderView::killTerminalEmulator()
{
    slotTerminalEmulator(false);    // hide the docking widget
    krToggleTerminal->setChecked(false);
}


void KrusaderView::profiles(QString profileName)
{
    ProfileManager profileManager("Panel");
    profileManager.hide();
    connect(&profileManager, SIGNAL(saveToProfile(QString)), this, SLOT(savePanelProfiles(QString)));
    connect(&profileManager, SIGNAL(loadFromProfile(QString)), this, SLOT(loadPanelProfiles(QString)));
    if (profileName.isEmpty())
        profileManager.profilePopup();
    else
        profileManager.loadProfile(profileName);
}

void KrusaderView::loadPanelProfiles(QString group)
{
    KConfigGroup ldg(krConfig, group);
    MAIN_VIEW->leftMng->loadSettings(&ldg, "Left Tabs");
    MAIN_VIEW->rightMng->loadSettings(&ldg, "Right Tabs");
    MAIN_VIEW->leftMng->setActiveTab(ldg.readEntry("Left Active Tab", 0));
    MAIN_VIEW->rightMng->setActiveTab(ldg.readEntry("Right Active Tab", 0));
    if (ldg.readEntry("Left Side Is Active", true))
        MAIN_VIEW->left->gui->slotFocusOnMe();
    else
        MAIN_VIEW->right->gui->slotFocusOnMe();
}

void KrusaderView::savePanelProfiles(QString group)
{
    KConfigGroup svr(krConfig, group);

    MAIN_VIEW->leftMng->saveSettings(&svr, "Left Tabs", false);
    svr.writeEntry("Left Active Tab", MAIN_VIEW->leftMng->activeTab());
    MAIN_VIEW->rightMng->saveSettings(&svr, "Right Tabs", false);
    svr.writeEntry("Right Active Tab", MAIN_VIEW->rightMng->activeTab());
    svr.writeEntry("Left Side Is Active", ACTIVE_PANEL->gui->isLeft());
}

void KrusaderView::toggleVerticalMode()
{
    if (horiz_splitter->orientation() == Qt::Vertical) {
        horiz_splitter->setOrientation(Qt::Horizontal);
        KrActions::actVerticalMode->setText(i18n("Vertical Mode"));
        KrActions::actVerticalMode->setIcon(KIcon("view-split-top-bottom"));
    } else {
        horiz_splitter->setOrientation(Qt::Vertical);
        KrActions::actVerticalMode->setText(i18n("Horizontal Mode"));
        KrActions::actVerticalMode->setIcon(KIcon("view-split-left-right"));
    }
}

void KrusaderView::saveSettings(KConfigGroup &cfg)
{
    cfg.writeEntry("Left Active Tab", leftMng->activeTab());
    cfg.writeEntry("Right Active Tab", rightMng->activeTab());
    cfg.writeEntry("Left Side Is Active", ACTIVE_PANEL->gui->isLeft());
    leftMng->saveSettings(&cfg, "Left Tab Bar");
    rightMng->saveSettings(&cfg, "Right Tab Bar");
}

#include "krusaderview.moc"
