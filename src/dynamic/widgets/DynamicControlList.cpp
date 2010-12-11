/****************************************************************************************
 * Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>                                    *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "DynamicControlList.h"
#include <QLayout>
#include <QLabel>
#include "DynamicControlWidget.h"
#include <QPaintEvent>
#include <QPainter>

using namespace Tomahawk;

DynamicControlList::DynamicControlList()
    : AnimatedWidget()
    , m_layout( new QVBoxLayout )
    , m_summaryWidget( 0 )
{
    init();
}

DynamicControlList::DynamicControlList( AnimatedSplitter* parent )
    : AnimatedWidget( parent )
    , m_layout( new QVBoxLayout )
    , m_summaryWidget( 0 )
{
    init();
}

DynamicControlList::DynamicControlList( const QList< dyncontrol_ptr >& controls, AnimatedSplitter* parent)
    : AnimatedWidget(parent)
    , m_layout( new QVBoxLayout )
    , m_summaryWidget( 0 )
{
    init();
    setControls( controls );
}

DynamicControlList::~DynamicControlList()
{

}

void 
DynamicControlList::init()
{
    setLayout( m_layout );
    m_layout->setMargin( 0 );
    m_layout->setSpacing( 0 );
//     setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Ignored );
    
    m_summaryWidget = new QWidget( this );
    // TODO replace
//     m_summaryWidget->setMinimumHeight( 24 );
//     m_summaryWidget->setMaximumHeight( 24 );
    m_summaryWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    m_summaryWidget->setLayout( new QVBoxLayout );
    m_summaryWidget->layout()->setMargin( 0 );
    m_summaryWidget->layout()->addWidget( new QLabel( "replace me plz", m_summaryWidget ) );
    
    setHiddenSize( m_summaryWidget->size() );
    
    emit showWidget();
}

void 
DynamicControlList::setControls(const QList< dyncontrol_ptr >& controls)
{
    foreach( const dyncontrol_ptr& control, controls ) {
        m_controls << new DynamicControlWidget( control, false, false, this );
    }
    onShown( this );
}

void 
DynamicControlList::onHidden( QWidget* w )
{
    if( w != this )
        return;
    
    AnimatedWidget::onHidden( w );
    
    foreach( DynamicControlWidget* control, m_controls ) {
        m_layout->removeWidget( control );
        control->hide();
    }
    m_layout->addWidget( m_summaryWidget );
    m_summaryWidget->show();
}

void 
DynamicControlList::onShown( QWidget* w )
{
    if( w != this )
        return;
    
    AnimatedWidget::onShown( w );
    
    m_layout->removeWidget( m_summaryWidget );
    m_summaryWidget->hide();
    foreach( DynamicControlWidget* control, m_controls ) {
        m_layout->addWidget( control );
        control->show();
        control->setShowPlusButton( control == m_controls.last() );
        control->setShowCollapseButton( control == m_controls.last() );
    }
}

void DynamicControlList::paintEvent(QPaintEvent* )
{
}