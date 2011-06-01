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

#ifndef ALBUMMODEL_H
#define ALBUMMODEL_H

#include <QAbstractItemModel>
#include <QPixmap>

#include "album.h"
#include "collection.h"
#include "playlistinterface.h"
#include "database/databasecommand_allalbums.h"

#include "albumitem.h"

#include "dllmacro.h"

class QMetaData;

class DLLEXPORT AlbumModel : public QAbstractItemModel
{
Q_OBJECT

public:
    explicit AlbumModel( QObject* parent = 0 );
    virtual ~AlbumModel();

    virtual QModelIndex index( int row, int column, const QModelIndex& parent ) const;
    virtual QModelIndex parent( const QModelIndex& child ) const;

    virtual bool isReadOnly() const { return true; }

    virtual int trackCount() const { return rowCount( QModelIndex() ); }
    virtual int albumCount() const { return rowCount( QModelIndex() ); }

    virtual int rowCount( const QModelIndex& parent ) const;
    virtual int columnCount( const QModelIndex& parent ) const;

    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role ) const;

    virtual void removeIndex( const QModelIndex& index );
    virtual void removeIndexes( const QList<QModelIndex>& indexes );

    virtual QMimeData* mimeData( const QModelIndexList& indexes ) const;
    virtual QStringList mimeTypes() const;
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const;

    void addCollection( const Tomahawk::collection_ptr& collection );
    void addFilteredCollection( const Tomahawk::collection_ptr& collection, unsigned int amount, DatabaseCommand_AllAlbums::SortOrder order );

    virtual QString title() const { return m_title; }
    virtual QString description() const { return m_description; }
    virtual void setTitle( const QString& title ) { m_title = title; }
    virtual void setDescription( const QString& description ) { m_description = description; }

    AlbumItem* itemFromIndex( const QModelIndex& index ) const
    {
        if ( index.isValid() )
            return static_cast<AlbumItem*>( index.internalPointer() );
        else
        {
            return m_rootItem;
        }
    }

public slots:
    virtual void setRepeatMode( PlaylistInterface::RepeatMode /*mode*/ ) {}
    virtual void setShuffled( bool /*shuffled*/ ) {}

signals:
    void repeatModeChanged( PlaylistInterface::RepeatMode mode );
    void shuffleModeChanged( bool enabled );

    void trackCountChanged( unsigned int tracks );

protected:

private slots:
    void onAlbumsAdded( const QList<Tomahawk::album_ptr>& albums );
    void onCoverArtDownloaded();
    void onDataChanged();

private:
    QPersistentModelIndex m_currentIndex;
    AlbumItem* m_rootItem;
    QPixmap m_defaultCover;

    QString m_title;
    QString m_description;
};

#endif // ALBUMMODEL_H
