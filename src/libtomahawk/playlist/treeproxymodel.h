/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TREEPROXYMODEL_H
#define TREEPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "playlistinterface.h"
#include "treemodel.h"

#include "dllmacro.h"

class DLLEXPORT TreeProxyModel : public QSortFilterProxyModel, public PlaylistInterface
{
Q_OBJECT

public:
    explicit TreeProxyModel( QObject* parent = 0 );

    virtual TreeModel* sourceModel() const { return m_model; }
    virtual void setSourceModel( TreeModel* sourceModel );

    virtual QPersistentModelIndex currentItem() const { return mapFromSource( m_model->currentItem() ); }
    virtual void setCurrentItem( const QModelIndex& index ) { m_model->setCurrentItem( mapToSource( index ) ); }

    virtual QList<Tomahawk::query_ptr> tracks() { Q_ASSERT( FALSE ); QList<Tomahawk::query_ptr> queries; return queries; }

    virtual int unfilteredTrackCount() const { return sourceModel()->rowCount( QModelIndex() ); }
    virtual int trackCount() const { return rowCount( QModelIndex() ); }
    virtual int albumCount() const { return rowCount( QModelIndex() ); }

    virtual void removeIndex( const QModelIndex& index );
    virtual void removeIndexes( const QList<QModelIndex>& indexes );

    virtual Tomahawk::result_ptr siblingItem( int direction );

    virtual void setFilter( const QString& pattern );

    virtual PlaylistInterface::RepeatMode repeatMode() const { return m_repeatMode; }
    virtual bool shuffled() const { return m_shuffled; }
    virtual PlaylistInterface::ViewMode viewMode() const { return PlaylistInterface::Tree; }

    TreeModelItem* itemFromIndex( const QModelIndex& index ) const { return sourceModel()->itemFromIndex( index ); }

signals:
    void repeatModeChanged( PlaylistInterface::RepeatMode mode );
    void shuffleModeChanged( bool enabled );

    void trackCountChanged( unsigned int tracks );
    void sourceTrackCountChanged( unsigned int tracks );

    void filterChanged( const QString& filter );

public slots:
    virtual void setRepeatMode( RepeatMode mode ) { m_repeatMode = mode; emit repeatModeChanged( mode ); }
    virtual void setShuffled( bool enabled ) { m_shuffled = enabled; emit shuffleModeChanged( enabled ); }

protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex& sourceParent ) const;
    bool lessThan( const QModelIndex& left, const QModelIndex& right ) const;

private:
    QString textForItem( TreeModelItem* item ) const;

    TreeModel* m_model;
    RepeatMode m_repeatMode;
    bool m_shuffled;
};

#endif // TREEPROXYMODEL_H
