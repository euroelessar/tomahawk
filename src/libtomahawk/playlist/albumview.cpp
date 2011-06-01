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

#include "albumview.h"

#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>

#include "audio/audioengine.h"

#include "tomahawksettings.h"
#include "albumitemdelegate.h"
#include "viewmanager.h"

using namespace Tomahawk;


AlbumView::AlbumView( QWidget* parent )
    : QListView( parent )
    , m_model( 0 )
    , m_proxyModel( 0 )
//    , m_delegate( 0 )
{
    setDragEnabled( true );
    setDropIndicatorShown( false );
    setDragDropOverwriteMode( false );
    setUniformItemSizes( true );
    setSpacing( 20 );

    setResizeMode( Adjust );
    setViewMode( IconMode );
    setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

    setProxyModel( new AlbumProxyModel( this ) );

    connect( this, SIGNAL( doubleClicked( QModelIndex ) ), SLOT( onItemActivated( QModelIndex ) ) );
}


AlbumView::~AlbumView()
{
    qDebug() << Q_FUNC_INFO;
}


void
AlbumView::setProxyModel( AlbumProxyModel* model )
{
    m_proxyModel = model;
    setItemDelegate( new AlbumItemDelegate( this, m_proxyModel ) );

    QListView::setModel( m_proxyModel );
}


void
AlbumView::setModel( QAbstractItemModel* model )
{
    Q_UNUSED( model );
    qDebug() << "Explicitly use setAlbumModel instead";
    Q_ASSERT( false );
}


void
AlbumView::setAlbumModel( AlbumModel* model )
{
    m_model = model;

    if ( m_proxyModel )
    {
        m_proxyModel->setSourceAlbumModel( m_model );
        m_proxyModel->sort( 0 );
    }

    connect( m_proxyModel, SIGNAL( filterChanged( QString ) ), SLOT( onFilterChanged( QString ) ) );

    setAcceptDrops( false );
}


void
AlbumView::onItemActivated( const QModelIndex& index )
{
    AlbumItem* item = m_model->itemFromIndex( m_proxyModel->mapToSource( index ) );
    if ( item )
    {
//        qDebug() << "Result activated:" << item->album()->tracks().first()->toString() << item->album()->tracks().first()->results().first()->url();
//        APP->audioEngine()->playItem( item->album().data(), item->album()->tracks().first()->results().first() );

        ViewManager::instance()->show( item->album() );
    }
}


void
AlbumView::dragEnterEvent( QDragEnterEvent* event )
{
    qDebug() << Q_FUNC_INFO;
    QListView::dragEnterEvent( event );
}


void
AlbumView::dragMoveEvent( QDragMoveEvent* event )
{
    QListView::dragMoveEvent( event );
}


void
AlbumView::dropEvent( QDropEvent* event )
{
    QListView::dropEvent( event );
}


void
AlbumView::paintEvent( QPaintEvent* event )
{
    QListView::paintEvent( event );
}


void
AlbumView::onFilterChanged( const QString& )
{
    if ( selectedIndexes().count() )
        scrollTo( selectedIndexes().at( 0 ), QAbstractItemView::PositionAtCenter );
}


void
AlbumView::startDrag( Qt::DropActions supportedActions )
{
    Q_UNUSED( supportedActions );
}


// Inspired from dolphin's draganddrophelper.cpp
QPixmap
AlbumView::createDragPixmap( int itemCount ) const
{
    Q_UNUSED( itemCount );
    return QPixmap();
}
