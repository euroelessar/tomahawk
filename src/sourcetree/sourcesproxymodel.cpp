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

#include "sourcesproxymodel.h"

#include <QDebug>
#include <QTreeView>

#include "sourcesmodel.h"
#include "sourcetree/items/collectionitem.h"


SourcesProxyModel::SourcesProxyModel( SourcesModel* model, QObject* parent )
    : QSortFilterProxyModel( parent )
    , m_model( model )
    , m_filtered( false )
{
    setDynamicSortFilter( true );
    setSortRole( SourcesModel::SortRole );

    setSourceModel( model );

    connect( model, SIGNAL( askForExpand( QModelIndex ) ), this, SLOT( askedToExpand( QModelIndex ) ) );
    connect( model, SIGNAL( selectRequest( QModelIndex ) ), this, SLOT( selectRequested( QModelIndex ) ) );
}


void
SourcesProxyModel::showOfflineSources()
{
    m_filtered = false;
    invalidateFilter();
}


void
SourcesProxyModel::hideOfflineSources()
{
    m_filtered = true;
    invalidateFilter();
}


bool
SourcesProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex& sourceParent ) const
{
    if ( !m_filtered )
        return true;


    CollectionItem* sti = qobject_cast< CollectionItem* >( m_model->data( sourceModel()->index( sourceRow, 0, sourceParent ), SourcesModel::SourceTreeItemRole ).value< SourceTreeItem* >() );
    if ( sti )
    {
        if ( sti->source().isNull() || sti->source()->isOnline() )
            return true;
        else
            return false;
    }
    // accept rows that aren't sources
    return true;
}

void
SourcesProxyModel::selectRequested( const QModelIndex& idx )
{
    qDebug() << "asking for select from idx:" << idx << idx.model()->metaObject()->className();
    emit selectRequest( mapFromSource( idx ) );
}

