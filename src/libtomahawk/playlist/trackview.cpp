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

#include "trackview.h"

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>

#include "audio/audioengine.h"
#include "utils/tomahawkutils.h"
#include "widgets/overlaywidget.h"
#include "dynamic/widgets/LoadingSpinner.h"

#include "trackheader.h"
#include "viewmanager.h"
#include "queueview.h"
#include "trackmodel.h"
#include "trackproxymodel.h"
#include "track.h"
#include "globalactionmanager.h"

using namespace Tomahawk;


TrackView::TrackView( QWidget* parent )
    : QTreeView( parent )
    , m_model( 0 )
    , m_proxyModel( 0 )
    , m_delegate( 0 )
    , m_header( new TrackHeader( this ) )
    , m_overlay( new OverlayWidget( this ) )
    , m_loadingSpinner( new LoadingSpinner( this ) )
    , m_resizing( false )
    , m_dragging( false )
{
    setSortingEnabled( false );
    setAlternatingRowColors( true );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );
    setDragEnabled( true );
    setDropIndicatorShown( false );
    setDragDropMode( QAbstractItemView::InternalMove );
    setDragDropOverwriteMode( false );
    setAllColumnsShowFocus( true );
    setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
    setRootIsDecorated( false );
    setUniformRowHeights( true );
    setMinimumWidth( 300 );
//    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    setHeader( m_header );

#ifndef Q_WS_WIN
    QFont f = font();
    f.setPointSize( f.pointSize() - 1 );
    setFont( f );
#endif

#ifdef Q_WS_MAC
    f.setPointSize( f.pointSize() - 2 );
    setFont( f );
#endif

    QAction* createLinkAction = new QAction( tr( "Copy track link" ), this );
    connect( createLinkAction, SIGNAL( triggered( bool ) ), this, SLOT( copyLink() ) );
    addAction( createLinkAction );

    connect( this, SIGNAL( doubleClicked( QModelIndex ) ), SLOT( onItemActivated( QModelIndex ) ) );
}


TrackView::~TrackView()
{
    qDebug() << Q_FUNC_INFO;

    delete m_header;
}


void
TrackView::setGuid( const QString& guid )
{
    m_guid = guid;
}


void
TrackView::setProxyModel( TrackProxyModel* model )
{
    m_proxyModel = model;

    m_delegate = new PlaylistItemDelegate( this, m_proxyModel );
    setItemDelegate( m_delegate );

    QTreeView::setModel( m_proxyModel );
}


void
TrackView::setModel( QAbstractItemModel* model )
{
    Q_UNUSED( model );
    qDebug() << "Explicitly use setTrackModel instead";
    Q_ASSERT( false );
}


void
TrackView::setTrackModel( TrackModel* model )
{
    m_model = model;

    if ( m_proxyModel )
    {
        m_proxyModel->setSourceTrackModel( m_model );
    }

    connect( m_model, SIGNAL( itemSizeChanged( QModelIndex ) ), SLOT( onItemResized( QModelIndex ) ) );
    connect( m_model, SIGNAL( loadingStarted() ), m_loadingSpinner, SLOT( fadeIn() ) );
    connect( m_model, SIGNAL( loadingFinished() ), m_loadingSpinner, SLOT( fadeOut() ) );

    connect( m_proxyModel, SIGNAL( filterChanged( QString ) ), SLOT( onFilterChanged( QString ) ) );

    setAcceptDrops( true );
}


void
TrackView::onItemActivated( const QModelIndex& index )
{
    TrackModelItem* item = m_model->itemFromIndex( m_proxyModel->mapToSource( index ) );
    if ( item && item->query()->numResults() )
    {
        qDebug() << "Result activated:" << item->query()->toString() << item->query()->results().first()->url();
        m_proxyModel->setCurrentItem( index );
        AudioEngine::instance()->playItem( m_proxyModel, item->query()->results().first() );
    }
}


void
TrackView::keyPressEvent( QKeyEvent* event )
{
    qDebug() << Q_FUNC_INFO;
    QTreeView::keyPressEvent( event );

    if ( !model() )
        return;

    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
        onItemActivated( currentIndex() );
    }
}


void
TrackView::onItemResized( const QModelIndex& index )
{
    qDebug() << Q_FUNC_INFO;
    m_delegate->updateRowSize( index );
}

void
TrackView::playItem()
{
    onItemActivated( m_contextMenuIndex );
}


void
TrackView::addItemsToQueue()
{
    foreach( const QModelIndex& idx, selectedIndexes() )
    {
        if ( idx.column() )
            continue;

        TrackModelItem* item = model()->itemFromIndex( proxyModel()->mapToSource( idx ) );
        if ( item && item->query()->numResults() )
        {
            ViewManager::instance()->queue()->model()->append( item->query() );
            ViewManager::instance()->showQueue();
        }
    }
}


void
TrackView::resizeEvent( QResizeEvent* event )
{
    QTreeView::resizeEvent( event );
    m_header->checkState();
}


void
TrackView::dragEnterEvent( QDragEnterEvent* event )
{
    qDebug() << Q_FUNC_INFO;
    QTreeView::dragEnterEvent( event );

    if ( event->mimeData()->hasFormat( "application/tomahawk.query.list" )
        || event->mimeData()->hasFormat( "application/tomahawk.plentry.list" )
        || event->mimeData()->hasFormat( "application/tomahawk.result.list" ) )
    {
        m_dragging = true;
        m_dropRect = QRect();

        qDebug() << "Accepting Drag Event";
        event->acceptProposedAction();
    }
}


void
TrackView::dragMoveEvent( QDragMoveEvent* event )
{
    QTreeView::dragMoveEvent( event );

    if ( model()->isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( event->mimeData()->hasFormat( "application/tomahawk.query.list" )
        || event->mimeData()->hasFormat( "application/tomahawk.plentry.list" )
        || event->mimeData()->hasFormat( "application/tomahawk.result.list" ) )
    {
        setDirtyRegion( m_dropRect );
        const QPoint pos = event->pos();
        const QModelIndex index = indexAt( pos );

        if ( index.isValid() )
        {
            const QRect rect = visualRect( index );
            m_dropRect = rect;

            // indicate that the item will be inserted above the current place
            const int gap = 5; // FIXME constant
            m_dropRect = QRect( rect.left(), rect.top() - gap / 2, rect.width(), gap );

            event->acceptProposedAction();
        }

        setDirtyRegion( m_dropRect );
    }
}


void
TrackView::dropEvent( QDropEvent* event )
{
    QTreeView::dropEvent( event );

    if ( event->isAccepted() )
    {
        qDebug() << "Ignoring accepted event!";
    }
    else
    {
        if ( event->mimeData()->hasFormat( "application/tomahawk.query.list" ) )
        {
            const QPoint pos = event->pos();
            const QModelIndex index = indexAt( pos );

            qDebug() << "Drop Event accepted at row:" << index.row();
            event->acceptProposedAction();

            if ( !model()->isReadOnly() )
            {
                model()->dropMimeData( event->mimeData(), event->proposedAction(), index.row(), 0, index.parent() );
            }
        }
        else if ( event->mimeData()->hasFormat( "application/tomahawk.result.list" ) )
        {
            const QPoint pos = event->pos();
            const QModelIndex index = indexAt( pos );

            qDebug() << "Drop Event accepted at row:" << index.row();
            event->acceptProposedAction();

            if ( !model()->isReadOnly() )
            {
                model()->dropMimeData( event->mimeData(), event->proposedAction(), index.row(), 0, index.parent() );
            }
        }
    }

    m_dragging = false;
}


void
TrackView::paintEvent( QPaintEvent* event )
{
    QTreeView::paintEvent( event );
    QPainter painter( viewport() );

    if ( m_dragging )
    {
        // draw drop indicator
        {
            // draw indicator for inserting items
            QBrush blendedBrush = viewOptions().palette.brush( QPalette::Normal, QPalette::Highlight );
            QColor color = blendedBrush.color();

            const int y = ( m_dropRect.top() + m_dropRect.bottom() ) / 2;
            const int thickness = m_dropRect.height() / 2;

            int alpha = 255;
            const int alphaDec = alpha / ( thickness + 1 );
            for ( int i = 0; i < thickness; i++ )
            {
                color.setAlpha( alpha );
                alpha -= alphaDec;
                painter.setPen( color );
                painter.drawLine( 0, y - i, width(), y - i );
                painter.drawLine( 0, y + i, width(), y + i );
            }
        }
    }
}


void
TrackView::onFilterChanged( const QString& )
{
    if ( selectedIndexes().count() )
        scrollTo( selectedIndexes().at( 0 ), QAbstractItemView::PositionAtCenter );

    if ( !proxyModel()->filter().isEmpty() && !proxyModel()->trackCount() &&
         model()->trackCount() )
    {
        m_overlay->setText( tr( "Sorry, your filter '%1' did not match any results." ).arg( proxyModel()->filter() ) );
        m_overlay->show();
    }
    else
        if ( model()->trackCount() )
            m_overlay->hide();
}


void
TrackView::copyLink()
{
    TrackModelItem* item = model()->itemFromIndex( proxyModel()->mapToSource( contextMenuIndex() ) );
    if ( item && !item->query().isNull() )
    {
        GlobalActionManager::instance()->copyToClipboard( item->query() );
    }
}


void
TrackView::startDrag( Qt::DropActions supportedActions )
{
    QList<QPersistentModelIndex> pindexes;
    QModelIndexList indexes;
    foreach( const QModelIndex& idx, selectedIndexes() )
    {
        if ( ( m_proxyModel->flags( idx ) & Qt::ItemIsDragEnabled ) )
        {
            indexes << idx;
            pindexes << idx;
        }
    }

    if ( indexes.count() == 0 )
        return;

    qDebug() << "Dragging" << indexes.count() << "indexes";
    QMimeData* data = m_proxyModel->mimeData( indexes );
    if ( !data )
        return;

    QDrag* drag = new QDrag( this );
    drag->setMimeData( data );
    const QPixmap p = TomahawkUtils::createDragPixmap( indexes.count() );
    drag->setPixmap( p );
    drag->setHotSpot( QPoint( -20, -20 ) );

    Qt::DropAction action = drag->exec( supportedActions, Qt::CopyAction );
    if ( action == Qt::MoveAction )
    {
        m_proxyModel->removeIndexes( pindexes );
    }
}
