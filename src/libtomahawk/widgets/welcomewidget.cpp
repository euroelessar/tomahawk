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

#include "welcomewidget.h"
#include "ui_welcomewidget.h"

#include "audio/audioengine.h"
#include "utils/tomahawkutils.h"

#include "viewmanager.h"
#include "playlist/playlistmodel.h"

#include "widgets/overlaywidget.h"

#include "sourcelist.h"
#include "tomahawksettings.h"

#include <QPainter>
#include "welcomeplaylistmodel.h"

#define HISTORY_TRACK_ITEMS 25
#define HISTORY_PLAYLIST_ITEMS 5
#define HISTORY_RESOLVING_TIMEOUT 2500


WelcomeWidget::WelcomeWidget( QWidget* parent )
    : QWidget( parent )
    , ui( new Ui::WelcomeWidget )
{
    ui->setupUi( this );

    WelcomePlaylistModel* model = new WelcomePlaylistModel( this );
    model->setMaxPlaylists( HISTORY_PLAYLIST_ITEMS );

    ui->playlistWidget->setItemDelegate( new PlaylistDelegate() );
    ui->playlistWidget->setModel( model );
    ui->playlistWidget->overlay()->resize( 380, 86 );
    ui->tracksView->overlay()->setEnabled( false );

    connect( model, SIGNAL( emptinessChanged( bool) ), this, SLOT( updatePlaylists() ) );

    m_tracksModel = new PlaylistModel( ui->tracksView );
    ui->tracksView->setPlaylistModel( m_tracksModel );
    m_tracksModel->loadHistory( Tomahawk::source_ptr(), HISTORY_TRACK_ITEMS );

    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL( timeout() ), SLOT( checkQueries() ) );

    connect( SourceList::instance(), SIGNAL( sourceAdded( Tomahawk::source_ptr ) ), SLOT( onSourceAdded( Tomahawk::source_ptr ) ) );

    connect( ui->playlistWidget, SIGNAL( activated( QModelIndex ) ), SLOT( onPlaylistActivated( QModelIndex ) ) );
    connect( AudioEngine::instance() ,SIGNAL( playlistChanged( PlaylistInterface* ) ), this, SLOT( updatePlaylists() ), Qt::QueuedConnection );
}


WelcomeWidget::~WelcomeWidget()
{
    delete ui;
}


void
WelcomeWidget::updatePlaylists()
{
//     ui->playlistWidget->clear();

//     QList<Tomahawk::playlist_ptr> playlists = TomahawkSettings::instance()->recentlyPlayedPlaylists();
/*
    foreach( const Tomahawk::playlist_ptr& playlist, playlists )
    {
        connect( playlist.data(), SIGNAL( revisionLoaded( Tomahawk::PlaylistRevision ) ), SLOT( refresh() ) );

        PlaylistWidgetItem* item = new PlaylistWidgetItem( playlist );
        ui->playlistWidget->addItem( item );
        item->setData( Qt::DisplayRole, playlist->title() );
    }*/

    int num = ui->playlistWidget->model()->rowCount( QModelIndex() );
    if ( num == 0 )
    {
        ui->playlistWidget->overlay()->setText( tr( "You have not played any playlists yet." ) );
        ui->playlistWidget->overlay()->show();
    }
    else
        ui->playlistWidget->overlay()->hide();
}

void
WelcomeWidget::onSourceAdded( const Tomahawk::source_ptr& source )
{
    connect( source.data(), SIGNAL( playbackFinished( Tomahawk::query_ptr ) ), SLOT( onPlaybackFinished( Tomahawk::query_ptr ) ) );
}


void
WelcomeWidget::checkQueries()
{
    m_timer->stop();
    m_tracksModel->ensureResolved();
}


void
WelcomeWidget::onPlaybackFinished( const Tomahawk::query_ptr& query )
{
    m_tracksModel->insert( 0, query );

    if ( m_tracksModel->trackCount() > HISTORY_TRACK_ITEMS )
        m_tracksModel->remove( HISTORY_TRACK_ITEMS );

    if ( m_timer->isActive() )
        m_timer->stop();
    m_timer->start( HISTORY_RESOLVING_TIMEOUT );
}


void
WelcomeWidget::onPlaylistActivated( const QModelIndex& item )
{
    qDebug() << Q_FUNC_INFO;

    Tomahawk::playlist_ptr pl = item.data( WelcomePlaylistModel::PlaylistRole ).value< Tomahawk::playlist_ptr >();
    if( Tomahawk::dynplaylist_ptr dynplaylist = pl.dynamicCast< Tomahawk::DynamicPlaylist >() )
        ViewManager::instance()->show( dynplaylist );
    else
        ViewManager::instance()->show( pl );
}


void
WelcomeWidget::changeEvent( QEvent* e )
{
    QWidget::changeEvent( e );
    switch ( e->type() )
    {
        case QEvent::LanguageChange:
            ui->retranslateUi( this );
            break;

        default:
            break;
    }
}

QSize
PlaylistDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_UNUSED( option );
    Q_UNUSED( index );
    return QSize( 0, 64 );
}


void
PlaylistDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption( &opt, QModelIndex() );
    qApp->style()->drawControl( QStyle::CE_ItemViewItem, &opt, painter );

    if ( option.state & QStyle::State_Selected )
    {
        opt.palette.setColor( QPalette::Text, opt.palette.color( QPalette::HighlightedText ) );
    }

    painter->save();
    painter->setRenderHint( QPainter::Antialiasing );
    painter->setPen( opt.palette.color( QPalette::Text ) );

    QTextOption to;
    to.setAlignment( Qt::AlignCenter );
    QFont font = opt.font;
    QFont boldFont = opt.font;
    boldFont.setBold( true );

    painter->drawPixmap( option.rect.adjusted( 10, 13, -option.rect.width() + 48, -13 ), m_playlistIcon );

    painter->drawText( option.rect.adjusted( 56, 26, -100, -8 ), index.data( WelcomePlaylistModel::ArtistRole ).toString() );

    QString trackCount = tr( "%1 tracks" ).arg( index.data( WelcomePlaylistModel::TrackCountRole ).toString() );
    painter->drawText( option.rect.adjusted( option.rect.width() - 96, 2, 0, -2 ), trackCount, to );

    painter->setFont( boldFont );
    painter->drawText( option.rect.adjusted( 56, 6, -100, -option.rect.height() + 20 ), index.data().toString() );

    painter->restore();
}


PlaylistWidget::PlaylistWidget( QWidget* parent )
    : QListView( parent )
{
    m_overlay = new OverlayWidget( this );
}
