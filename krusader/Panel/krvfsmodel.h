/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KRVFSMODEL_H
#define KRVFSMODEL_H

// QtCore
#include <QAbstractListModel>
// QtGui
#include <QFont>

#include "krinterview.h"
#include "krsort.h"


class vfile;
class KrViewProperties;

class KrVfsModel: public QAbstractListModel
{
    Q_OBJECT

public:
    KrVfsModel(KrInterView *);
    virtual ~KrVfsModel();

    inline bool ready() const {
        return _ready;
    }
    void populate(const QList<vfile*> &files, vfile *dummy);
    QModelIndex addItem(vfile *);
    QModelIndex removeItem(vfile *);
    void updateItem(vfile *vf);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void setExtensionEnabled(bool exten) {
        _extensionEnabled = exten;
    }
    inline const KrViewProperties * properties() const {
        return _view->properties();
    }
    virtual void sort() {
        sort(lastSortOrder(), lastSortDir());
    }
    void clear();
    QList<vfile*> vfiles() {
        return _vfiles;
    }
    vfile * vfileAt(const QModelIndex &index);
    vfile *dummyVfile() const {
        return _dummyVfile;
    }
    const QModelIndex & vfileIndex(const vfile *);
    const QModelIndex & nameIndex(const QString &);
    const QModelIndex & indexFromUrl(const QUrl &url);
    virtual Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE;
    void emitChanged() {
        emit layoutChanged();
    }
    Qt::SortOrder lastSortDir() const {
        return (properties()->sortOptions & KrViewProperties::Descending) ? Qt::DescendingOrder : Qt::AscendingOrder;
    }
    int lastSortOrder() const {
        return properties()->sortColumn;
    }
    void setAlternatingTable(bool altTable) {
        _alternatingTable = altTable;
    }

public slots:
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) Q_DECL_OVERRIDE;

protected:
    virtual KrSort::LessThanFunc lessThanFunc() const {
        return KrSort::itemLessThan;
    }
    virtual KrSort::LessThanFunc greaterThanFunc() const {
        return KrSort::itemGreaterThan;
    }
    virtual QVariant customSortData(vfile *vf) const {
        Q_UNUSED(vf);
        return QVariant();
    }
    KrSort::Sorter createSorter();
    QString nameWithoutExtension(const vfile * vf, bool checkEnabled = true) const;


    QList<vfile*>               _vfiles;
    QHash<vfile *, QModelIndex> _vfileNdx;
    //TODO: use url index instead of name index
    QHash<QString, QModelIndex> _nameNdx;
    bool                        _extensionEnabled;
    KrInterView                 * _view;
    vfile *                     _dummyVfile;
    bool                        _ready;
    QFont                       _defaultFont;
    bool                        _justForSizeHint;
    bool                        _alternatingTable;
};

#endif // __krvfsmodel__
