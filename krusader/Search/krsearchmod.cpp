/***************************************************************************
                                krsearchmod.cpp
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

                                                    S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "krsearchmod.h"
#include "../VFS/krquery.h"
#include "../krusader.h"
#include "../resources.h"
#include "../VFS/vfile.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krarchandler.h"

#include <klocale.h>
#include <qdir.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <klargefile.h>
#include <kurlrequesterdlg.h> 

#include <kmimetype.h>

KRSearchMod::KRSearchMod( const KRQuery* q )
{
  stopSearch = false; /// ===> added
  query = new KRQuery( *q );
  query->normalize();
   
  remote_vfs = 0;
}

KRSearchMod::~KRSearchMod()
{
  delete query;
  if( remote_vfs )
    delete remote_vfs;
}

void KRSearchMod::start()
{
  unScannedUrls.clear();
  scannedUrls.clear();
  
  // search every dir that needs to be searched
  for ( unsigned int i = 0; i < query->whereToSearch.count(); ++i )
      scanURL( query->whereToSearch [ i ] );
  
  emit finished();
}

void KRSearchMod::stop()
{
  stopSearch = true;
}

void KRSearchMod::scanURL( KURL url )
{
  if( stopSearch ) return;
  
  unScannedUrls.push( url );    
  while ( !unScannedUrls.isEmpty() )
  {
    KURL urlToCheck = unScannedUrls.pop();
    
    if( stopSearch ) return;
    
    if ( query->whereNotToSearch.contains( urlToCheck ) )
      continue;
    
    if( scannedUrls.contains( urlToCheck ) )
      continue;    
    scannedUrls.push( urlToCheck );
    
    emit searching( urlToCheck.prettyURL(0,KURL::StripFileProtocol) );
    
    if ( urlToCheck.isLocalFile() )
      scanLocalDir( urlToCheck );
    else
      scanRemoteDir( urlToCheck );
  
    qApp->processEvents(); // do a last one, in case passes%50 != 0
  }
}
  
void KRSearchMod::scanLocalDir( KURL urlToScan )
{
  int passes = 0;
  const int NO_OF_PASSES = 50;
    
  QString dir = urlToScan.path( 1 );

  DIR* d = opendir( dir.local8Bit() );
  if ( !d ) return ;

  struct dirent* dirEnt;

  while ( ( dirEnt = readdir( d ) ) != NULL )
  {
    QString name = QString::fromLocal8Bit( dirEnt->d_name );
    
    // we dont scan the ".",".." enteries
    if ( name == "." || name == ".." ) continue;

    KDE_struct_stat stat_p;
    KDE_lstat( ( dir + name ).local8Bit(), &stat_p );
    
    KURL url = vfs::fromPathOrURL( dir + name );
    
    QString mime = QString::null;
    if ( query->inArchive || !query->type.isEmpty() )
      mime = KMimeType::findByURL( url, stat_p.st_mode, true, false ) ->name();

    // creating a vfile object for matching with krquery    
    vfile * vf = new vfile(name, (KIO::filesize_t)stat_p.st_size, KRpermHandler::mode2QString(stat_p.st_mode),
                           stat_p.st_mtime, S_ISLNK(stat_p.st_mode), stat_p.st_uid, stat_p.st_gid,
                           mime, "", stat_p.st_mode);
    vf->vfile_setUrl( url );
    
    if ( query->recurse )
    {
      if ( S_ISLNK( stat_p.st_mode ) && query->followLinks )
        unScannedUrls.push( vfs::fromPathOrURL( QDir( dir + name ).canonicalPath() ) );
      else if ( S_ISDIR( stat_p.st_mode ) )
        unScannedUrls.push( url );
    }
    if ( query->inArchive )
    {
      QString type = mime.right( 4 );
      if ( mime.contains( "-rar" ) ) type = "-rar";

      if ( KRarcHandler::arcSupported( type ) )
      {
        KURL archiveURL = url;
        
        if ( type == "-tbz" || type == "-tgz" || type == "tarz" || type == "-tar" )
          archiveURL.setProtocol( "tar" );
        else
          archiveURL.setProtocol( "krarc" );
          
        unScannedUrls.push( archiveURL );
      }
    }
    
    if( query->match( vf ) )
    {
      // if we got here - we got a winner
      results.append( dir + name );
      emit found( name, dir, ( KIO::filesize_t ) stat_p.st_size, stat_p.st_mtime, KRpermHandler::mode2QString( stat_p.st_mode ) );
      if ( passes++ % NO_OF_PASSES == 0 ) qApp->processEvents();
    }
    delete vf;
  }
  // clean up
  closedir( d );
}

void KRSearchMod::scanRemoteDir( KURL url )
{
  int passes = 0;
  const int NO_OF_PASSES = 50;

  if( remote_vfs == 0 )
    remote_vfs = new ftp_vfs( 0 );
    
  if ( !remote_vfs->vfs_refresh( url ) ) return ;

  for ( vfile * vf = remote_vfs->vfs_getFirstFile(); vf != 0 ; vf = remote_vfs->vfs_getNextFile() )
  {
    QString name = vf->vfile_getName();

    if ( query->recurse )
    {      
      if( ( vf->vfile_isSymLink() && query->followLinks ) || vf->vfile_isDir() )
      {
        KURL recurseURL = remote_vfs->vfs_getOrigin();
        recurseURL.addPath( name );
        unScannedUrls.push( recurseURL );
      }
    }
    
    if( query->match( vf ) )
    {
      // if we got here - we got a winner
      results.append( remote_vfs->vfs_getOrigin().prettyURL( 1 ) + name );
      emit found( name, remote_vfs->vfs_getOrigin().prettyURL( -1 ), vf->vfile_getSize(), vf->vfile_getTime_t(), vf->vfile_getPerm() );
      if ( passes++ % NO_OF_PASSES == 0 ) qApp->processEvents();
    }
  }
}

#include "krsearchmod.moc"
