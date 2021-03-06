/*
 *
 *  Copyright (c) 2013
 *  name : mhogo mchungu
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "walletmanager.h"
#include "ui_walletmanager.h"
#include "task.h"

walletmanager::walletmanager( const QString& icon,QDialog * parent ) :QDialog( parent ),m_ui( 0 ),m_wallet( 0 )
{
	m_icon = QString( ":/%1" ).arg( icon ) ;
}

void walletmanager::buildGUI()
{
	m_ui = new Ui::walletmanager ;
	m_ui->setupUi( this ) ;

	this->setFixedSize( this->size() ) ;
	this->setWindowFlags( Qt::Window | Qt::Dialog ) ;
	this->setWindowIcon( QIcon( m_icon ) ) ;

	connect( m_ui->pushButtonAccountAdd,SIGNAL( clicked() ),this,SLOT( pushButtonAdd() ) ) ;
	connect( m_ui->pushButtonClose,SIGNAL( clicked() ),this,SLOT( pushButtonClose() ) ) ;

	connect( m_ui->tableWidget,SIGNAL( itemClicked( QTableWidgetItem * ) ),
		 this,SLOT( tableItemClicked( QTableWidgetItem * ) ) ) ;
	connect( m_ui->tableWidget,SIGNAL( currentItemChanged( QTableWidgetItem *,QTableWidgetItem * ) ),this,
		 SLOT( tableItemChanged( QTableWidgetItem *,QTableWidgetItem * ) ) ) ;

	m_ui->tableWidget->setColumnWidth( 0,171 ) ;
	m_ui->tableWidget->setColumnWidth( 1,171 ) ;
	m_ui->tableWidget->setColumnWidth( 2,171 ) ;

	m_table = m_ui->tableWidget ;

	m_ui->pushButtonAccountAdd->setFocus() ;

	m_ui->groupBox->setTitle( QString( "" ) ) ;

	this->installEventFilter( this ) ;
}

void walletmanager::ShowUI()
{
	m_action = walletmanager::showAccountInfo ;
	m_wallet = configurationoptionsdialog::secureStorageSystem() ;
	m_wallet->setInterfaceObject( this ) ;
	m_wallet->setImage( m_icon ) ;
	QString s = configurationoptionsdialog::walletName( m_wallet->backEnd() ) ;
	m_wallet->open( s,QString( "qCheckGMail" ) ) ;
}

void walletmanager::getAccounts( void )
{
	m_action = walletmanager::getAccountInfo ;
	m_wallet = configurationoptionsdialog::secureStorageSystem() ;
	m_wallet->setInterfaceObject( this ) ;
	QString s = configurationoptionsdialog::walletName( m_wallet->backEnd() ) ;
	m_wallet->open( s,QString( "qCheckGMail" ) ) ;
}

void walletmanager::changeWalletPassword()
{
	m_wallet = configurationoptionsdialog::secureStorageSystem() ;
	m_wallet->setInterfaceObject( this ) ;
	m_wallet->setImage( m_icon ) ;
	QString s = configurationoptionsdialog::walletName( m_wallet->backEnd() ) ;
	m_wallet->changeWalletPassWord( s,QString( "qCheckGMail" ) ) ;
}

void walletmanager::walletpassWordChanged( bool passwordChanged )
{
	Q_UNUSED( passwordChanged ) ;
	this->deleteLater() ;
}

void walletmanager::walletIsOpen( bool walletOpened )
{
	if( walletOpened ){
		Task * t ;
		switch( m_action ){
		case walletmanager::showAccountInfo :

			this->buildGUI() ;
			this->disableAll() ;
			this->show() ;

			t = new Task( m_wallet,&m_accounts ) ;
			connect( t,SIGNAL( taskFinished( int ) ),this,SLOT( taskComplete( int ) ) ) ;
			t->start( Task::showAccountInfo ) ;

			break ;
		case walletmanager::getAccountInfo :

			t = new Task( m_wallet,&m_accounts ) ;
			connect( t,SIGNAL( taskFinished( int ) ),this,SLOT( taskComplete( int ) ) ) ;
			t->start( Task::getAccountInfo ) ;

			break ;
		default:
			/*
			 * we dont get here
			 */
			this->deleteLater() ;
		}
	}else{
		this->deleteLater() ;
	}
}

walletmanager::~walletmanager()
{
	emit walletmanagerClosed() ;
	m_wallet->deleteLater() ;
	delete m_ui ;
}

void walletmanager::closeEvent( QCloseEvent * e )
{
	e->ignore() ;
	this->HideUI() ;
}

bool walletmanager::eventFilter( QObject * watched,QEvent * event )
{
	if( watched == this ){
		if( event->type() == QEvent::KeyPress ){
			QKeyEvent * keyEvent = static_cast< QKeyEvent* >( event ) ;
			if( keyEvent->key() == Qt::Key_Escape ){
				this->HideUI() ;
				return true ;
			}
		}
	}

	return false ;
}

void walletmanager::HideUI()
{
	emit getAccountsInfo( m_accounts ) ;
	this->hide() ;
	this->deleteLater() ;
}

void walletmanager::pushButtonAdd()
{
	this->disableAll() ;
	addaccount * ac = new addaccount( this ) ;
	connect( ac,SIGNAL( addAccount( QString,QString,QString,QString ) ),
		 this,SLOT( addAccount( QString,QString,QString,QString ) ) ) ;
	connect( ac,SIGNAL( cancelSignal() ),this,SLOT( enableAll() ) ) ;
	ac->ShowUI() ;
}

void walletmanager::pushButtonClose()
{
	this->HideUI() ;
}

void walletmanager::tableItemClicked( QTableWidgetItem * item )
{
	m_deleteRow = item->row() ;

	QMenu m ;

	QAction * ac = new QAction( &m ) ;
	ac->setText( tr( "delete entry" ) ) ;
	connect( ac,SIGNAL( triggered() ),this,SLOT( deleteRow() ) ) ;

	m.addAction( ac ) ;

	ac = new QAction( &m ) ;
	ac->setText( tr( "edit entry" ) ) ;
	connect( ac,SIGNAL( triggered() ),this,SLOT( editEntry() ) ) ;

	m.addAction( ac ) ;

	m.exec( QCursor::pos() ) ;
}

void walletmanager::deleteRow()
{
	QTableWidgetItem * item = m_table->currentItem() ;
	m_row = item->row() ;
	m_accName = m_table->item( m_row,0 )->text() ;

	QMessageBox msg( this ) ;
	msg.setText( tr( "are you sure you want to delete \"%1\" account?" ).arg( m_accName ) ) ;
	msg.addButton( tr( "yes" ),QMessageBox::YesRole ) ;
	QPushButton * no_button = msg.addButton( tr( "no" ),QMessageBox::NoRole ) ;
	msg.setDefaultButton( no_button ) ;

	this->disableAll() ;

	msg.exec() ;

	if( msg.clickedButton() != no_button ){
		if( m_row < m_accounts.size() && m_row < m_table->rowCount() ){
			Task * t = new Task( m_wallet,m_accName ) ;
			connect( t,SIGNAL( taskFinished( int ) ),this,SLOT( taskComplete( int ) ) ) ;
			t->start( Task::deleteAccount ) ;
		}else{
			this->enableAll() ;
		}
	}else{
		this->enableAll() ;
	}
}

void walletmanager::enableAll()
{
	m_ui->groupBox->setEnabled( true ) ;
	m_ui->pushButtonAccountAdd->setEnabled( true ) ;
	m_ui->pushButtonClose->setEnabled( true ) ;
	m_ui->tableWidget->setEnabled( true ) ;
}

void walletmanager::disableAll()
{
	m_ui->groupBox->setEnabled( false ) ;
	m_ui->pushButtonAccountAdd->setEnabled( false ) ;
	m_ui->pushButtonClose->setEnabled( false ) ;
	m_ui->tableWidget->setEnabled( false ) ;
}

void walletmanager::editEntry()
{
	int row = m_table->currentRow() ;

	QString accName        = m_table->item( row,0 )->text() ;
	QString accPassword    = this->getPassWordFromAccount( accName ) ;
	QString accDisplayName = m_table->item( row,1 )->text() ;
	QString accLabels      = m_table->item( row,2 )->text() ;

	addaccount * ac = new addaccount( row,accName,accPassword,accDisplayName,accLabels,this ) ;
	connect( ac,SIGNAL( editAccount( int,QString,QString,QString,QString ) ),
		 this,SLOT( editAccount( int,QString,QString,QString,QString ) ) ) ;
	connect( ac,SIGNAL( cancelSignal() ),this,SLOT( enableAll() ) ) ;
	ac->ShowUI() ;
}

void walletmanager::addAccount( QString accName,QString accPassword,
				 QString accDisplayName,QString accLabels )
{
	m_accName        = accName ;
	m_accPassWord    = accPassword ;
	m_accLabels      = accLabels ;
	m_accDisplayName = accDisplayName ;

	Task * t = new Task( m_wallet,m_accName,m_accPassWord,m_accLabels,m_accDisplayName ) ;
	connect( t,SIGNAL( taskFinished( int ) ),this,SLOT( taskComplete( int ) ) ) ;
	t->start( Task::addAccount ) ;
}

void walletmanager::editAccount( int row,QString accName,QString accPassword,
				  QString accDisplayName,QString accLabels )
{
	m_row            = row ;
	m_accName        = accName ;
	m_accPassWord    = accPassword ;
	m_accLabels      = accLabels ;
	m_accDisplayName = accDisplayName ;

	Task * t = new Task( m_wallet,m_accName,m_accPassWord,m_accLabels,m_accDisplayName ) ;
	connect( t,SIGNAL( taskFinished( int ) ),this,SLOT( taskComplete( int ) ) ) ;
	t->start( Task::editAccount ) ;
}

void walletmanager::taskComplete( int r )
{
	int row ;
	int j ;
	QTableWidgetItem * item ;

	switch( Task::action( r ) ){
	case Task::editAccount :

		m_accounts.replace( m_row,accounts( m_accName,m_accPassWord,m_accDisplayName,m_accLabels ) ) ;
		m_table->item( m_row,0 )->setText( m_accName ) ;
		m_table->item( m_row,1 )->setText( m_accDisplayName ) ;
		m_table->item( m_row,2 )->setText( m_accLabels ) ;
		this->enableAll() ;

		break ;
	case Task::addAccount :

		m_accounts.append( accounts( m_accName,m_accPassWord,m_accDisplayName,m_accLabels ) ) ;

		row = m_table->rowCount() ;
		m_table->insertRow( row ) ;

		item = new QTableWidgetItem() ;
		item->setText( m_accName ) ;
		item->setTextAlignment( Qt::AlignCenter ) ;
		m_table->setItem( row,0,item ) ;

		item = new QTableWidgetItem() ;
		item->setText( m_accDisplayName ) ;
		item->setTextAlignment( Qt::AlignCenter ) ;
		m_table->setItem( row,1,item ) ;

		item = new QTableWidgetItem() ;
		item->setText( m_accLabels ) ;
		item->setTextAlignment( Qt::AlignCenter ) ;
		m_table->setItem( row,2,item ) ;

		this->selectRow( row,true ) ;
		this->enableAll() ;

		break ;
	case Task::deleteAccount :

		m_accounts.remove( m_row ) ;
		m_table->removeRow( m_row ) ;
		this->enableAll() ;

		break ;
	case Task::getAccountInfo :

		emit getAccountsInfo( m_accounts ) ;
		this->deleteLater() ;

		break ;
	case Task::showAccountInfo :

		j = m_accounts.size() ;
		row = -1 ;

		for( int i = 0 ; i < j ; i++ ){
			row = m_table->rowCount() ;

			m_table->insertRow( row ) ;

			item = new QTableWidgetItem() ;
			item->setText( m_accounts.at( i ).accountName() ) ;
			item->setTextAlignment( Qt::AlignCenter ) ;
			m_table->setItem( row,0,item ) ;

			item = new QTableWidgetItem() ;
			item->setText( m_accounts.at( i ).displayName() ) ;
			item->setTextAlignment( Qt::AlignCenter ) ;
			m_table->setItem( row,1,item ) ;

			item = new QTableWidgetItem() ;
			item->setText( m_accounts.at( i ).labels() ) ;
			item->setTextAlignment( Qt::AlignCenter ) ;
			m_table->setItem( row,2,item ) ;
		}

		this->selectRow( row,true ) ;
		this->enableAll() ;

		break ;
	default:
		/*
		* we do not get here
		*/
		this->enableAll() ;
	}
}

void walletmanager::selectRow( int row,bool highlight )
{
	if( row >= 0 ){
		int j = m_table->columnCount() ;
		for( int i = 0 ; i < j ; i++ ){
			m_table->item( row,i )->setSelected( highlight ) ;
		}

		if( highlight && j > 0 ){
			m_table->setCurrentCell( row,j - 1 ) ;
		}
	}
}

const QString& walletmanager::getPassWordFromAccount( const QString& accName )
{
	int j = m_accounts.size() ;
	for( int i = 0 ; i < j ; i++ ){
		if( m_accounts.at( i ).accountName() == accName ){
			return m_accounts.at( i ).passWord() ;
		}
	}

	static QString shouldNotGetHere ;
	return shouldNotGetHere ;
}

void walletmanager::tableItemChanged( QTableWidgetItem * current,QTableWidgetItem * previous )
{
	if( current && previous ){
		if( current->row() == previous->row() ){
			this->selectRow( current->row(),true ) ;
			return ;
		}
	}
	if( current ){
		this->selectRow( current->row(),true ) ;
	}
	if( previous ){
		this->selectRow( previous->row(),false ) ;
	}
}
