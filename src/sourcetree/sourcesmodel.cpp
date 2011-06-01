
/*
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

#include "sourcetree/sourcesmodel.h"

#include "sourcetree/items/sourcetreeitem.h"
#include "sourcetree/items/collectionitem.h"
#include "sourcetree/items/genericpageitems.h"
#include "sourcelist.h"
#include "playlist.h"
#include "collection.h"
#include "source.h"
#include "viewmanager.h"

#include <boost/bind.hpp>

#include <QMimeData>
#include <QSize>

using namespace Tomahawk;

SourcesModel::SourcesModel( QObject* parent )
    : QAbstractItemModel( parent )
    , m_rootItem( 0 )
    , m_viewPageDelayedCacheItem( 0 )
{
    m_rootItem = new SourceTreeItem( this, 0, Invalid );

    appendItem( source_ptr() );

    // add misc children of root node
    new GenericPageItem( this, m_rootItem->children().at( 0 ), tr( "Recently Played" ), QIcon( RESPATH "images/recently-played.png" ),
                                                   boost::bind( &ViewManager::showWelcomePage, ViewManager::instance() ),
                                                   boost::bind( &ViewManager::welcomeWidget, ViewManager::instance() )
                                                 );

    onSourcesAdded( SourceList::instance()->sources() );

    connect( SourceList::instance(), SIGNAL( sourceAdded( Tomahawk::source_ptr ) ), SLOT( onSourceAdded( Tomahawk::source_ptr ) ) );
    connect( SourceList::instance(), SIGNAL( sourceRemoved( Tomahawk::source_ptr ) ), SLOT( onSourceRemoved( Tomahawk::source_ptr ) ) );
    connect( ViewManager::instance(), SIGNAL( viewPageActivated( Tomahawk::ViewPage* ) ), this, SLOT( viewPageActivated( Tomahawk::ViewPage* ) ) );
}


SourcesModel::~SourcesModel()
{
    delete m_rootItem;
}


QString
SourcesModel::rowTypeToString( RowType type )
{
    switch ( type )
    {
        case Collection:
            return tr( "Collection" );

        case StaticPlaylist:
            return tr( "Playlist" );

        case AutomaticPlaylist:
            return tr( "Automatic Playlist" );

        case Station:
            return tr( "Station" );

        default:
            return QString( "Unknown" );
    }
}


QVariant
SourcesModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() )
        return QVariant();

    switch( role )
    {
    case Qt::SizeHintRole:
        return QSize( 0, 18 );
    case SourceTreeItemRole:
        return QVariant::fromValue< SourceTreeItem* >( itemFromIndex( index ) );
    case SourceTreeItemTypeRole:
        return itemFromIndex( index )->type();
    case Qt::DisplayRole:
    case Qt::EditRole:
        return itemFromIndex( index )->text();
    case Qt::DecorationRole:
        return itemFromIndex( index )->icon();
    case SourcesModel::SortRole:
        return itemFromIndex( index )->peerSortValue();
    }
    return QVariant();
}


int
SourcesModel::columnCount( const QModelIndex& ) const
{
    return 1;
}


int
SourcesModel::rowCount( const QModelIndex& parent ) const
{
    if( !parent.isValid() ) {
        return m_rootItem->children().count();
    }
//     qDebug() << "ASKING FOR AND RETURNING ROWCOUNT:" << parent.row() << parent.column() << parent.internalPointer() << itemFromIndex( parent )->children().count() << itemFromIndex( parent )->text();
    return itemFromIndex( parent )->children().count();
}


QModelIndex
SourcesModel::parent( const QModelIndex& child ) const
{
//     qDebug() << Q_FUNC_INFO << child;
    if( !child.isValid() ) {
        return QModelIndex();
    }

    SourceTreeItem* node = itemFromIndex( child );
    SourceTreeItem* parent = node->parent();
    if( parent == m_rootItem )
        return QModelIndex();

    return createIndex( rowForItem( parent ), 0, parent );
}


QModelIndex
SourcesModel::index( int row, int column, const QModelIndex& parent ) const
{
//     qDebug() << "INDEX:" << row << column << parent;
    if( row < 0 || column < 0 )
        return QModelIndex();

    if( hasIndex( row, column, parent ) ) {
        SourceTreeItem *parentNode = itemFromIndex( parent );
        SourceTreeItem *childNode = parentNode->children().at( row );
//         qDebug() << "Making index with parent:" << parentNode->text() << "and index:" << childNode->text();
        return createIndex( row, column, childNode );
    }

    return QModelIndex();

}


bool
SourcesModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    SourceTreeItem* item = itemFromIndex( index );
    return item->setData( value, role );
}


QStringList
SourcesModel::mimeTypes() const
{
    QStringList types;
    types << "application/tomahawk.query.list";
    types << "application/tomahawk.result.list";
    return types;
}


QMimeData*
SourcesModel::mimeData( const QModelIndexList& ) const
{
    // TODO
    return new QMimeData();
}


bool
SourcesModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
    SourceTreeItem* item = 0;
    qDebug() << "Got mime data dropped:" << row << column << parent << itemFromIndex( parent )->text();
    if( row == -1 && column == -1 )
        item = itemFromIndex( parent );
    else if( column == 0 )
        item = itemFromIndex( index( row, column, parent ) );
    else if( column == -1 ) // column is -1, that means the drop is happening "below" the indices. that means we actually want the one before it
        item = itemFromIndex( index( row - 1, 0, parent ) );

    Q_ASSERT( item );

    qDebug() << "Dropping on:" << item->text();
    return item->dropMimeData( data, action );
}


Qt::DropActions
SourcesModel::supportedDropActions() const
{
    return Qt::CopyAction;
}


Qt::ItemFlags
SourcesModel::flags( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        return itemFromIndex( index )->flags();
    } else {
        return 0;
    }
}


void
SourcesModel::appendItem( const Tomahawk::source_ptr& source )
{
    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    // append to end
    new CollectionItem( this, m_rootItem, source );

    endInsertRows();
}


bool
SourcesModel::removeItem( const Tomahawk::source_ptr& source )
{
    qDebug() << "Removing source item from SourceTree:" << source->friendlyName();

    QModelIndex idx;
    int rows = rowCount();
    for ( int row = 0; row < rows; row++ )
    {
        QModelIndex idx = index( row, 0, QModelIndex() );
        CollectionItem* item = static_cast< CollectionItem* >( idx.internalPointer() );
        if ( item && item->source() == source )
        {
            qDebug() << "Found removed source item:" << item->source()->userName();
            beginRemoveRows( QModelIndex(), row, row );
            m_rootItem->removeChild( item );
            endRemoveRows();

//             onItemOffline( idx );

            delete item;
            return true;
        }
    }

    return false;
}


void
SourcesModel::viewPageActivated( Tomahawk::ViewPage* page )
{
    if ( m_sourceTreeLinks.contains( page ) )
    {
        Q_ASSERT( m_sourceTreeLinks[ page ] );
        qDebug() << "Got view page activated for itemL:" << m_sourceTreeLinks[ page ]->text();
        QModelIndex idx = indexFromItem( m_sourceTreeLinks[ page ] );
        Q_ASSERT( idx.isValid() );

        emit selectRequest( idx );
    } else {
        m_viewPageDelayedCacheItem = page;
    }
}


void
SourcesModel::loadSources()
{
    QList<source_ptr> sources = SourceList::instance()->sources();

    foreach( const source_ptr& source, sources )
        appendItem( source );
}


void
SourcesModel::onSourcesAdded( const QList<source_ptr>& sources )
{
    foreach( const source_ptr& source, sources )
        appendItem( source );
}


void
SourcesModel::onSourceAdded( const source_ptr& source )
{
    appendItem( source );
}


void
SourcesModel::onSourceRemoved( const source_ptr& source )
{
    removeItem( source );
}


void
SourcesModel::itemUpdated()
{
    Q_ASSERT( qobject_cast< SourceTreeItem* >( sender() ) );
    SourceTreeItem* item = qobject_cast< SourceTreeItem* >( sender() );

    if( !item )
        return;

    QModelIndex idx = indexFromItem( item );
    emit dataChanged( idx, idx );
}


void
SourcesModel::onItemRowsAddedBegin( int first, int last )
{

    Q_ASSERT( qobject_cast< SourceTreeItem* >( sender() ) );
    SourceTreeItem* item = qobject_cast< SourceTreeItem* >( sender() );

    if( !item )
        return;

    QModelIndex idx = indexFromItem( item );
    beginInsertRows( idx, first, last );
}


void
SourcesModel::onItemRowsAddedDone()
{
    Q_ASSERT( qobject_cast< SourceTreeItem* >( sender() ) );

    endInsertRows();
}


void
SourcesModel::onItemRowsRemovedBegin( int first, int last )
{
    Q_ASSERT( qobject_cast< SourceTreeItem* >( sender() ) );
    SourceTreeItem* item = qobject_cast< SourceTreeItem* >( sender() );

    if( !item )
        return;

    QModelIndex idx = indexFromItem( item );
    beginRemoveRows( idx, first, last );
}


void
SourcesModel::onItemRowsRemovedDone()
{
    Q_ASSERT( qobject_cast< SourceTreeItem* >( sender() ) );

    endRemoveRows();
}


void
SourcesModel::linkSourceItemToPage( SourceTreeItem* item, ViewPage* p )
{
    // TODO handle removal
    m_sourceTreeLinks[ p ] = item;

    if( m_viewPageDelayedCacheItem == p )
        emit selectRequest( indexFromItem( item ) );

    m_viewPageDelayedCacheItem = 0;
}

SourceTreeItem*
SourcesModel::itemFromIndex( const QModelIndex& idx ) const
{
    if( !idx.isValid() )
        return m_rootItem;

    Q_ASSERT( idx.internalPointer() );

    return reinterpret_cast< SourceTreeItem* >( idx.internalPointer() );
}


QModelIndex
SourcesModel::indexFromItem( SourceTreeItem* item ) const
{
    if( !item || !item->parent() ) // should never happen..
        return QModelIndex();

    // reconstructs a modelindex from a sourcetreeitem that is somewhere in the tree
    // traverses the item to the root node, then rebuilds the qmodeindices from there back down
    // each int is the row of that item in the parent.
    /**
     * In this diagram, if the \param item is G, childIndexList will contain [0, 2, 0]
     *
     *    A
     *      D
     *      E
     *      F
     *        G
     *        H
     *    B
     *    C
     *
     **/
    QList< int > childIndexList;
    SourceTreeItem* curItem = item;
    while( curItem != m_rootItem ) {
        childIndexList << rowForItem( curItem );

        curItem = curItem->parent();
    }
//     qDebug() << "build child index list:" << childIndexList;
    // now rebuild the qmodelindex we need
    QModelIndex idx;
    for( int i = childIndexList.size() - 1; i >= 0 ; i-- ) {
        idx = index( childIndexList[ i ], 0, idx );
    }
//     qDebug() << "Got index from item:" << idx << idx.data( Qt::DisplayRole ).toString();
//     qDebug() << "parent:" << idx.parent();
    return idx;
}


int
SourcesModel::rowForItem( SourceTreeItem* item ) const
{
    return item->parent()->children().indexOf( item );
}

void
SourcesModel::itemSelectRequest( SourceTreeItem* item )
{
    emit selectRequest( indexFromItem( item ) );
}
