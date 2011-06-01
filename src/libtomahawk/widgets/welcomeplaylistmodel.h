/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Leo Franchi <leo.franchi@kdab.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef WELCOMEPLAYLISTMODEL_H
#define WELCOMEPLAYLISTMODEL_H

#include <QModelIndex>

#include "playlist.h"


class WelcomePlaylistModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ItemRoles
    { ArtistRole = Qt::UserRole, TrackCountRole, PlaylistRole };

    explicit WelcomePlaylistModel( QObject* parent = 0 );

    void setMaxPlaylists( unsigned int max ) { m_maxPlaylists = max; }

    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;

signals:
    void emptinessChanged( bool isEmpty );

private slots:
    void playlistChanged( PlaylistInterface* );
    void onSourceAdded( const Tomahawk::source_ptr& source );
    void onPlaylistsRemoved( QList<Tomahawk::playlist_ptr> );
    void loadFromSettings();

    void plAdded( const Tomahawk::playlist_ptr& );

private:
    QList< Tomahawk::playlist_ptr > m_recplaylists;
    QHash< QString, Tomahawk::playlist_ptr > m_cached;
    mutable QHash< Tomahawk::playlist_ptr, QString > m_artists;

    unsigned int m_maxPlaylists;
    bool m_waitingForSome;
};

#endif // WELCOMEPLAYLISTMODEL_H
