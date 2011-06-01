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

#ifndef PLAYLISTINTERFACE_H
#define PLAYLISTINTERFACE_H

#include <QModelIndex>
#include <QWidget>

#include "typedefs.h"

#include "dllmacro.h"
#include "result.h"

class DLLEXPORT PlaylistInterface
{
public:
    enum RepeatMode { NoRepeat, RepeatOne, RepeatAll };
    enum ViewMode { Unknown, Tree, Flat, Album };

    PlaylistInterface( QObject* parent = 0 ) : m_object( parent ) {}
    virtual ~PlaylistInterface() {}

    virtual QList< Tomahawk::query_ptr > tracks() = 0;

    virtual int unfilteredTrackCount() const = 0;
    virtual int trackCount() const = 0;

    virtual Tomahawk::result_ptr previousItem() { return siblingItem( -1 ); }
    virtual Tomahawk::result_ptr nextItem() { return siblingItem( 1 ); }
    virtual Tomahawk::result_ptr siblingItem( int itemsAway ) = 0;

    virtual PlaylistInterface::RepeatMode repeatMode() const = 0;
    virtual bool shuffled() const = 0;
    virtual PlaylistInterface::ViewMode viewMode() const { return Unknown; }

    virtual QString filter() const { return m_filter; }
    virtual void setFilter( const QString& pattern ) { m_filter = pattern; }

    QObject* object() const { return m_object; }

public slots:
    virtual void setRepeatMode( RepeatMode mode ) = 0;
    virtual void setShuffled( bool enabled ) = 0;

signals:
    virtual void repeatModeChanged( PlaylistInterface::RepeatMode mode ) = 0;
    virtual void shuffleModeChanged( bool enabled ) = 0;
    virtual void trackCountChanged( unsigned int tracks ) = 0;
    virtual void sourceTrackCountChanged( unsigned int tracks ) = 0;

private:
    QObject* m_object;

    QString m_filter;
};

#endif // PLAYLISTINTERFACE_H
