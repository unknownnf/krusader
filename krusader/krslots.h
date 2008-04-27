/***************************************************************************
                                krslots.h
                            -------------------
   copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
   email                : krusader@users.sourceforge.net
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

                                                    H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/



#ifndef KRSLOTS_H
#define KRSLOTS_H

#include <qobject.h>
#include <kprocess.h>
#include <kio/netaccess.h>
// the 2 following #includes should go away with the ugly stubs on the bottom
#include "krusader.h"
#include "krusaderview.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "BookMan/krbookmarkbutton.h"
#include "GUI/kcmdline.h"
#include "GUI/dirhistorybutton.h"
#include "GUI/mediabutton.h"

class ListPanel;
class KUrl;

class KrProcess: public KProcess
{
  Q_OBJECT

  QString tmp1, tmp2;
  
public:
  KrProcess( QString in1, QString in2 )
  {
    tmp1 = in1;
    tmp2 = in2;
    connect(this,  SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processHasExited()));
  }

public slots:
  void processHasExited()
  {
    if( !tmp1.isEmpty() )
      KIO::NetAccess::removeTempFile( tmp1 );
    if( !tmp2.isEmpty() )
      KIO::NetAccess::removeTempFile( tmp2 );
    deleteLater();
  }
};

class KRslots : public QObject {
    Q_OBJECT

  public:
    enum compareMode { full } ;

    KRslots(QObject *parent): QObject(parent) {}
    ~KRslots() {}

  public slots:
    void createChecksum();
    void matchChecksum();
    void sendFileByEmail( QString filename );
    void compareContent();
    void compareContent( KUrl, KUrl );
    void rightclickMenu();
    void insertFileName(bool full_path);
    void rootKrusader();
    void swapPanels();
    void setBriefView();
    void setDetailedView();
    void toggleHidden();
    void toggleSwapSides();
	 void togglePopupPanel();
    void configToolbar();
    void configKeys();
    void toggleToolbar();
	 void toggleActionsToolbar();
    void toggleStatusbar();
    void toggleTerminal();
    void home();
    void root();
    void dirUp();
    void markAll();
    void unmarkAll();
    void markGroup();
	 void markGroup(const QString &, bool select);
    void unmarkGroup();
    void invert();
    void compareDirs();
    void compareSetup();
    /** called by actExec* actions to choose the built-in command line mode */
    void execTypeSetup();
    void refresh();
    void refresh( const KUrl& u );
    void properties();
    void back();
    void slotPack();
    void slotUnpack();
    void cut();
    void copy();
    void paste();
    void testArchive();
    void calcSpace();
    void FTPDisconnect();
    void allFilter();
    void execFilter();
    void customFilter();
    void newFTPconnection();
    void runKonfigurator( bool firstTime = false );
    void startKonfigurator() { runKonfigurator( false ); }
    void search();						 				// call the search module
    void locate();
    void homeTerminal();
    void sysInfo();
    void addBookmark();
    void runMountMan();
    void toggleFnkeys();
    void toggleCmdline();
    void changeTrashIcon();
    void multiRename();
    void openRightBookmarks() { RIGHT_PANEL->slotFocusOnMe(); RIGHT_PANEL->bookmarksButton->showMenu(); }
    void openLeftBookmarks() { LEFT_PANEL->slotFocusOnMe(); LEFT_PANEL->bookmarksButton->showMenu(); }
	 void openBookmarks() { ACTIVE_PANEL->bookmarksButton->showMenu(); }
    void bookmarkCurrent();
	 void openHistory() { ACTIVE_PANEL->historyButton->showMenu(); }
    void openLeftHistory() { LEFT_PANEL->historyButton->showMenu(); }
    void openRightHistory() { RIGHT_PANEL->historyButton->showMenu(); }
    void openMedia() { ACTIVE_PANEL->mediaButton->showMenu(); }
    void openLeftMedia() { LEFT_PANEL->mediaButton->showMenu(); }
    void openRightMedia() { RIGHT_PANEL->mediaButton->showMenu(); }
	 void syncPanels() {
	 	ListPanel *t = ACTIVE_PANEL;
		OTHER_FUNC->openUrl(ACTIVE_PANEL->virtualPath());
		t->slotFocusOnMe();
	 }
    void cmdlinePopup() { MAIN_VIEW->cmdLine->popup(); }
    void duplicateTab();
    void newTab(const KUrl& url = KUrl());
    void newTab(KrViewItem *item);
    void closeTab();
    void nextTab();
    void previousTab();
    void slotSplit();
    void slotCombine();
    void userMenu();
    void manageUseractions();
    void slotSynchronizeDirs( QStringList selected = QStringList() );
    void slotSyncBrowse();
    void slotDiskUsage();
    void slotLocationBar();
    void slotJumpBack();
    void slotSetJumpBack();
    void newSymlink() { ACTIVE_PANEL->func->krlink(true); }
    void updatePopupPanel(KrViewItem *);
		void windowActive(); // called when krusader's window becomes focussed
		void windowInactive(); // called when another application steals the focus
    // F2
    void terminal();
    // F3
    void view();
    // Shift F3
    void viewDlg();
    // F4
    void edit();
    // Shift F4
    void editDlg();
    // F5
    void copyFiles();
    // F6
    void moveFiles();
    // F7
    void mkdir();
    // F8
    void deleteFiles(bool reallyDelete=false);
    // F9
    void rename();

    // ugly stubs, remove later ?
    void slotCurrentChanged( QString p ) { MAIN_VIEW->slotCurrentChanged( p ); }
    void slotSetActivePanel( ListPanel *p ) { MAIN_VIEW->slotSetActivePanel( p ); }

    
    void jsConsole();
};

#endif
