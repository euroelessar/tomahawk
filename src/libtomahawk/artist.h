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

#ifndef TOMAHAWKARTIST_H
#define TOMAHAWKARTIST_H

#include <QObject>
#include <QSharedPointer>

#include "typedefs.h"

#include "playlistinterface.h"

#include "dllmacro.h"

namespace Tomahawk
{

class DLLEXPORT Artist : public QObject, public PlaylistInterface
{
Q_OBJECT

public:
    static artist_ptr get( unsigned int id, const QString& name );
    Artist( unsigned int id, const QString& name );

    Artist();
    virtual ~Artist();

    unsigned int id() const { return m_id; }
    QString name() const { return m_name; }

    virtual QList<Tomahawk::query_ptr> tracks();

    virtual int trackCount() const { return 0; }
    virtual int unfilteredTrackCount() const { return m_queries.count(); }

    virtual Tomahawk::result_ptr siblingItem( int itemsAway );

    virtual PlaylistInterface::RepeatMode repeatMode() const { return PlaylistInterface::NoRepeat; }
    virtual bool shuffled() const { return false; }

    virtual void setRepeatMode( PlaylistInterface::RepeatMode ) {}
    virtual void setShuffled( bool ) {}

    virtual void setFilter( const QString& /*pattern*/ ) {}

signals:
    void repeatModeChanged( PlaylistInterface::RepeatMode mode );
    void shuffleModeChanged( bool enabled );

    void tracksAdded( const QList<Tomahawk::query_ptr>& tracks );
    void trackCountChanged( unsigned int tracks );
    void sourceTrackCountChanged( unsigned int tracks );

private slots:
    void onTracksAdded( const QList<Tomahawk::query_ptr>& tracks );

private:
    unsigned int m_id;
    QString m_name;

    QList<Tomahawk::query_ptr> m_queries;
    unsigned int m_currentTrack;
};

}; // ns

#endif
