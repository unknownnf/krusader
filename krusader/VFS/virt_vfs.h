/***************************************************************************
                          virt_vfs.h  -  description
                             -------------------
    begin                : Fri Dec 5 2003
    copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VIRT_VFS_H
#define VIRT_VFS_H

#include "vfs.h"

/**
  *@author Shie Erlich & Rafi Yanai
  */

class virt_vfs : public vfs  {
Q_OBJECT
public: 
	virt_vfs(QObject* panel, bool quiet=false);
	~virt_vfs();
	
	/// Copy a file to the vfs (physical).
	void vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir = "");	
	/// Remove a file from the vfs (physical)
	void vfs_delFiles(QStringList *fileNames);	
	/// Return a list of URLs for multiple files	
	KURL::List* vfs_getFiles(QStringList* names);
	/// Return a URL to a single file	
	KURL vfs_getFile(const QString& name);
	/// Create a new directory
	void vfs_mkdir(const QString& name);
	/// Rename file
	void vfs_rename(const QString& fileName,const QString& newName);
	/// Calculate the amount of space occupied by a file or directory (recursive).
	void vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool * stop = 0);
	
	/// Return the VFS working dir
	QString vfs_workingDir(){ return QString::null; }

	/// Save the dictionary to file
	static bool save();
	/// Restore the dictionary from file
	static bool restore();
	
protected slots:
	void slotStatResult(KIO::Job *job);

protected:
	bool populateVfsList(const KURL& origin, bool showHidden);
	vfile* stat(const KURL& url);
	
	vfileDict  vfs_files;    //< List of pointers to vfile.
	static QDict<KURL::List> virtVfsDict;
	bool busy;
	QString path;
	KIO::UDSEntry entry;
};

#endif
