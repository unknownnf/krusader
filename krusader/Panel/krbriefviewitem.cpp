#include "krbriefviewitem.h"
#include "krbriefview.h"
#include "../krusaderview.h"
#include "../defaults.h"
#include "../kicons.h"
#include "listpanel.h"
#include <kconfig.h>
#include "krcolorcache.h"

#include <qpainter.h>
#include <q3pointarray.h>
#include <QPixmap>

#define PROPS	_viewProperties
#define VF	getVfile()

#ifdef FASTER
int KrBriefViewItem::expHeight = 0;
#endif // FASTER

KrBriefViewItem::KrBriefViewItem(KrBriefView *parent, Q3IconViewItem *after, vfile *vf):
	K3IconViewItem(parent, after), KrViewItem(vf, parent->properties()) {
#ifdef FASTER
	initiated = false;
	// get the expected height of an item - should be done only once
	if (expHeight == 0) {
		KConfigGroup svr( krConfig, "Look&Feel" );
  		expHeight = 2 + (svr.readEntry("Filelist Icon Size",_FilelistIconSize)).toInt();
	}
	if( PROPS->displayIcons )
		itemIcon = QPixmap( expHeight, expHeight );
#endif // FASTER

	// there's a special case, where if _vf is null, then we've got the ".." (updir) item
	// in that case, create a special vfile for that item, and delete it, if needed
	if (!_vf) {
		dummyVfile = true;
		_vf = new vfile("..", 0, "drw-r--r--", 0, false, 0, 0, QString(), QString(), 0);
		
		setText("..");
		if ( PROPS->displayIcons )
			setPixmap( FL_LOADICON( "go-up" ) );
		setSelectable( false );
		setDragEnabled( false );
		setDropEnabled( false );
#ifdef FASTER
		initiated = true;
#endif // FASTER
	}
	
	setRenameEnabled( false );
	setDragEnabled( true );
	setDropEnabled( true );
	repaintItem();
}


int KrBriefViewItem::compare(Q3IconViewItem *i ) const {
  bool ignoreCase = (PROPS->sortMode & KrViewProperties::IgnoreCase);

  KrBriefViewItem *other = (KrBriefViewItem *)i;
  int asc = iconView()->sortDirection() ? -1 : 1;

  bool thisDir = VF->vfile_isDir();
  bool otherDir = other->VF->vfile_isDir();

  // handle directory sorting
  if ( thisDir ){
    if ( !otherDir ) return 1*asc;
  } else if( otherDir ) return -1*asc;

  if ( isDummy() ) return 1*asc;
  if ( other->isDummy() ) return -1*asc;

  QString text0 = name();
  if (text0 == "..") return 1*asc;
  
  QString itext0 = other->name();
  if (itext0 == "..") return -1*asc;

  if( ignoreCase )
  {
    text0  = text0.toLower();
    itext0 = itext0.toLower();
  }

  if ( isHidden() ) {
    if ( !other->isHidden() ) return 1*asc;
  } else if ( other->isHidden() ) return -1*asc;

  if (!ignoreCase && !PROPS->localeAwareCompareIsCaseSensitive) {
    // sometimes, localeAwareCompare is not case sensative. in that case,
    // we need to fallback to a simple string compare (KDE bug #40131)
    return QString::compare(text0, itext0);
  } else return QString::localeAwareCompare(text0,itext0);
}

void KrBriefViewItem::paintItem(QPainter *p, const QColorGroup &cg) {
#ifdef FASTER
  if (!initiated && !dummyVfile) {
     // display an icon if needed
     initiated = true;
     if (PROPS->displayIcons)
       itemIcon =KrView::getIcon(_vf);
  }
#endif

  QColorGroup _cg(cg);

  KrColorItemType colorItemType;
  colorItemType.m_activePanel = (dynamic_cast<KrView *>(iconView()) == ACTIVE_PANEL->view);
  
  int gridX = iconView()->gridX();
  int xpos = x() / gridX;
  int ypos = y() / height();
  
  colorItemType.m_alternateBackgroundColor = (xpos & 1) ^ (ypos & 1) ;
  colorItemType.m_currentItem = (iconView()->currentItem() == this);
  colorItemType.m_selectedItem = isSelected();
  if (VF->vfile_isSymLink())
  {
     if (_vf->vfile_getMime() == "Broken Link !" )
        colorItemType.m_fileType = KrColorItemType::InvalidSymlink;
     else
        colorItemType.m_fileType = KrColorItemType::Symlink;
  }
  else if (VF->vfile_isDir())
     colorItemType.m_fileType = KrColorItemType::Directory;
  else if (VF->vfile_isExecutable())
     colorItemType.m_fileType = KrColorItemType::Executable;
  else
     colorItemType.m_fileType = KrColorItemType::File;

  KrColorGroup cols;
  KrColorCache::getColorCache().getColors(cols, colorItemType);
  _cg.setColor(QColorGroup::Base, cols.background());
  _cg.setColor(QColorGroup::Background, cols.background());
  _cg.setColor(QColorGroup::Text, cols.text());
  _cg.setColor(QColorGroup::HighlightedText, cols.highlightedText());
  _cg.setColor(QColorGroup::Highlight, cols.highlight());

  QColor background = isSelected() ? cols.highlight() : cols.background();

  if( background != iconView()->paletteBackgroundColor() )
  {
     p->save();
     p->setPen( Qt::NoPen );
     p->setBrush( QBrush( background ) );
     p->drawRect( rect() );
     p->restore();
  }

  Q3IconViewItem::paintItem(p, _cg);

  paintFocus( p, cg );
}

void KrBriefViewItem::paintFocus(QPainter *p, const QColorGroup &cg) {
  if ( ( iconView()->hasFocus() && this == iconView()->currentItem() ) || 
     ((KrBriefView *)iconView())->_currDragItem == this )
  {
      p->save();
      QPen pen( cg.shadow(), 0, Qt::DotLine );
      p->setPen( pen );

      // we manually draw the focus rect by points
      QRect rec = rect();
      Q3PointArray points( rec.right() - rec.left() + rec.bottom() - rec.top() + 4 );
      
      int ndx = 0;
      for( int x=rec.left(); x <= rec.right(); x+=2 )
      {
        points.setPoint( ndx++, x, rec.top() );
        points.setPoint( ndx++, x, rec.bottom() );
      }
      for( int y=rec.top(); y <= rec.bottom(); y+=2 )
      {
        points.setPoint( ndx++, rec.left(), y );
        points.setPoint( ndx++, rec.right(), y );
      }

      p->drawPoints( points );
      p->restore();

//    --- That didn't work with all themes
//    iconView()->style().drawPrimitive(QStyle::PE_FocusRect, p,
//       QRect( rect().x(), rect().y(), rect().width(), rect().height() ), cg,
//              QStyle::Style_Default, cg.base() );
  }
}

void KrBriefViewItem::itemHeightChanged() {
#ifdef FASTER
	expHeight = 0;
#endif // FASTER
}

void KrBriefViewItem::repaintItem()
{
   if ( dummyVfile ) return;

#ifndef FASTER
   if (PROPS->displayIcons)
     setPixmap(KrView::getIcon(_vf));
#endif // FASTER
   setText( _vf->vfile_getName() );
}

// for keeping the exact item heights...
void KrBriefViewItem::calcRect ( const QString & text_ )
{
   K3IconViewItem::calcRect( text_ );
   QRect rec = rect();
   
   int gridX = iconView()->gridX();
   int minX  = ( rec.x() / gridX ) * gridX;
   rec.setX( minX );
   rec.setWidth( gridX );
   rec.setHeight( expHeight );   
   setItemRect( rec );
   
   rec = pixmapRect();
   if( rec.height() > expHeight )
   {
     rec.setHeight( expHeight );
     setPixmapRect( rec );
   }   
   
   rec = textRect();
   if( rec.height() > expHeight )
   {
     rec.setHeight( expHeight );
     setTextRect( rec );
   }   
}
