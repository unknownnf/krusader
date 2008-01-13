/***************************************************************************
                                 krtreewidget.cpp
                             -------------------
    copyright            : (C) 2008+ by Csaba Karai
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krtreewidget.h"
#include <qevent.h>
#include <qheaderview.h>
#include <qapplication.h>
#include <qtooltip.h>

KrTreeWidget::KrTreeWidget( QWidget * parent )
{
  setRootIsDecorated( false );
  setSortingEnabled( true );
  setAllColumnsShowFocus(true);

  _stretchingColumn = -1;
}

bool KrTreeWidget::event ( QEvent * event )
{
  switch (event->type()) {
  case QEvent::Resize:
    {
      if( _stretchingColumn != -1 && columnCount() )
      {
        QResizeEvent *re = static_cast<QResizeEvent*>(event);
        if( re->size() == re->oldSize() )
          break;

        int ndx1 = header()->visualIndex( _stretchingColumn );
        int ndx2 = columnCount() - 1;
        header()->swapSections( ndx1, ndx2 );
        bool res = QTreeWidget::event( event );
        header()->swapSections( ndx1, ndx2 );
        return res;
      }
      break;
    }
  case QEvent::ToolTip:
    {
      QHelpEvent *he = static_cast<QHelpEvent*>(event);

      if( viewport() )
      {
        QPoint pos = viewport()->mapFromGlobal( he->globalPos() );

        QTreeWidgetItem * item = itemAt( pos );
        int column = columnAt( pos.x() );

        if ( item ) {
            QString tip = item->text( column );

            int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
            int requiredWidth = QFontMetrics( font() ).width( tip ) + 2 * textMargin;

            if( !tip.isEmpty() && ( columnWidth( column ) < requiredWidth ) )
              QToolTip::showText(he->globalPos(), tip, this);
            return true;
        }
      }
    }
    break;
  default:
    break;
  }
  return QTreeWidget::event( event );
}

void KrTreeWidget::mousePressEvent ( QMouseEvent * event )
{
  if( event->button() == Qt::RightButton )
  {
    QPoint pos = viewport()->mapFromGlobal( event->globalPos() );

    QTreeWidgetItem * item = itemAt( pos );
    int column = columnAt( pos.x() );
    emit itemRightClicked( item, column );
  }
  else
    QTreeWidget::mousePressEvent( event );
}

void KrTreeWidget::mouseReleaseEvent ( QMouseEvent * event )
{
  if( event->button() == Qt::RightButton )
    return;

  QTreeWidget::mouseReleaseEvent( event );
}