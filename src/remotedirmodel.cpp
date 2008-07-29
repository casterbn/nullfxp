// remotedirmodel.cpp --- 
// 
// Filename: remotedirmodel.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 日  5月 25 09:47:47 2008 (CST)
// Version: 
// Last-Updated: 
//           By: 
//     Update #: 0
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 

#include <cassert>
#include <QtCore>

#include "utils.h"
#include "globaloption.h"
#include "remotedirmodel.h"
#include "rfsdirnode.h"

RemoteDirModel::RemoteDirModel (  QObject *parent )
    :QAbstractItemModel ( parent )
{
    this->remote_dir_retrive_thread = new RemoteDirRetriveThread ( );
    QObject::connect ( this->remote_dir_retrive_thread,SIGNAL ( remote_dir_node_retrived ( directory_tree_item *,void * ) ),
                       this,SLOT ( slot_remote_dir_node_retrived ( directory_tree_item*,   void * ) ) );

    //
    QObject::connect ( this->remote_dir_retrive_thread,SIGNAL ( enter_remote_dir_retrive_loop() ),
                       this,SIGNAL ( enter_remote_dir_retrive_loop() ) );
    QObject::connect ( this->remote_dir_retrive_thread,SIGNAL ( leave_remote_dir_retrive_loop() ),
                       this,SIGNAL ( leave_remote_dir_retrive_loop() ) );
    
    //keep alive 相关设置
    this->keep_alive = true ;
    this->keep_alive_timer = new QTimer();
    this->keep_alive_interval = DEFAULT_KEEP_ALIVE_TIMEOUT ;
    this->keep_alive_timer->setInterval( this->keep_alive_interval );
    QObject::connect(this->keep_alive_timer,SIGNAL(timeout()),
                     this,SLOT( slot_keep_alive_time_out() ) );

}

void RemoteDirModel::set_ssh2_handler( void * ssh2_sess )
{
    this->remote_dir_retrive_thread->set_ssh2_handler(ssh2_sess);
}

void RemoteDirModel::set_user_home_path ( std::string user_home_path )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->user_home_path = user_home_path ;

    qDebug() <<" i know remote home path: "<< this->user_home_path.c_str() ;

    //Todo: 使用这个初始化路径来初始化这个树，而不是用默认的根路径 .
    //初始化目录案例：
    // 1.      /home/users/l/li/liuguangzhao
    // 2.      /root
    // 3.       /home/gzl

    this->tree_root = new  directory_tree_item ();

    //{创建初始化目录树
    directory_tree_item * first_item ;

    directory_tree_item * temp_parent_tree_item = 0 , * temp_tree_item =0 ;
    QString temp_strip_path , temp_path_name ;
    char   buff[PATH_MAX+1] = {0};
    char    buff2[PATH_MAX+1] = {0};
    char    buff3[PATH_MAX+1] = {0} ;
    char * sep_pos = 0 ,* pre_sep_pos ;
    strcpy ( buff,this->user_home_path.c_str() );
    strcpy ( buff2,this->user_home_path.c_str() );
    pre_sep_pos = buff ;
    while ( 1 )
    {
        sep_pos = strchr ( buff,'/' ) ;
        if ( sep_pos == NULL ) {
            temp_strip_path =  buff2 ;
            memset ( buff3,0,PATH_MAX+1 );
            strcpy ( buff3,buff2+ ( pre_sep_pos-buff ) +1 );
            temp_path_name =  buff3 ;
            temp_parent_tree_item = temp_tree_item ;
            temp_tree_item = new directory_tree_item();
        }else{
            *sep_pos = '&'; //将这个字符替换掉，防止重复查找这个位置
            if ( sep_pos == buff ) {
                strncpy ( buff3,buff2,int ( sep_pos-buff ) );
                temp_strip_path = buff3 ;
                temp_path_name =  "/" ;
                first_item   = new directory_tree_item();
                temp_tree_item = first_item ;
                temp_parent_tree_item = this->tree_root ;
            }else{
                strncpy ( buff3,buff2,int ( sep_pos-buff ) );
                temp_strip_path =  buff3 ;
                memset ( buff3,0,PATH_MAX+1 );
                strncpy ( buff3,buff2+ ( pre_sep_pos-buff ) +1, ( sep_pos-pre_sep_pos-1 ) );
                temp_path_name =  buff3 ;
                temp_parent_tree_item = temp_tree_item ;
                temp_tree_item = new directory_tree_item();
            }
        }
        qDebug() <<"distance to begin:   strip path:"<< temp_strip_path << "dir name:"<< temp_path_name ;
        //assign
        temp_tree_item->parent_item = temp_parent_tree_item ;
        temp_tree_item->row_number = 0 ;    //指的是此结点在父结点中的第几个结点，在这里预置的只能为0
        temp_tree_item->retrived = ( sep_pos == NULL ) ?0:1 ;  //半满结点
        temp_tree_item->strip_path = temp_strip_path;
        temp_tree_item->file_name = temp_path_name;
        temp_tree_item->file_size =  "0" ;
        temp_tree_item->file_type =  "drwxr-xr-x" ;
        temp_tree_item->prev_retr_flag = -1 ;
        temp_tree_item->attrib.permissions = 16877;//默认目录属性: drwxr-xr-x

        temp_parent_tree_item->child_items.insert(std::make_pair(0, temp_tree_item));

        //pre_sep_pos
        pre_sep_pos = sep_pos ;
        if ( sep_pos == NULL ) break ;
    }
    qDebug() <<" seach end :"<< buff ;

    this->tree_root->child_items.insert ( std::make_pair ( 0, first_item ) );
    this->tree_root->row_number = 0;
    
    //启动keep alive 功能。
    if(this->keep_alive ) {
        this->keep_alive_timer->start();
    }
}

RemoteDirModel::~RemoteDirModel()
{
    if(this->keep_alive_timer->isActive() ) {
        this->keep_alive_timer->stop();
    }
    delete this->keep_alive_timer ;
    
    if ( this->remote_dir_retrive_thread->isRunning() ) {
        //TODO 怎么能友好的结束,  现在这么做只能让程序不崩溃掉。
        qDebug() <<" remote_dir_retrive_thread is run , how stop ?";
        this->remote_dir_retrive_thread->terminate();
        //this->remote_dir_retrive_thread->wait();  //这个不好用
        delete this->remote_dir_retrive_thread ;        
    }else{
        delete this->remote_dir_retrive_thread ;
    }
    if( tree_root != 0 ) delete tree_root ;
    //TODO: 删除model中的现有数据, 已经实现，上一行
}

QModelIndex RemoteDirModel::index ( int row, int column, const QModelIndex &parent ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //qDebug() << "row :" << row << " column:" << column ;

    directory_tree_item  *parent_item;

    if ( !parent.isValid() )
        parent_item = this->tree_root ;
    else
        parent_item = static_cast<directory_tree_item*> ( parent.internalPointer() );

    directory_tree_item *child_item = 0;
    if ( parent_item->child_items.count ( row ) ==1 )
        child_item = parent_item->child_items[row];

    if ( child_item )
    {
        //qDebug()<< "createIndex ( row, column, child_item );"<< row << " " << column << " " << child_item ;
        return createIndex ( row, column, child_item );
    }
    else
    {
        //qDebug()<< "! child_item , QModelIndex();" ;
        return QModelIndex();
    }

    //return createIndex( row , column , 1 ) ;
}

QModelIndex RemoteDirModel::index ( const QString & path, int column  ) const
{
    if( path == "" ) {
        qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
        return this->createIndex(0,0,this->tree_root->child_items[0]  );
    } 

    QString absolutePath = QDir(path).absolutePath();
#ifdef Q_OS_WIN
    //absolutePath = absolutePath.toLower();
    // On Windows, "filename......." and "filename" are equivalent
    if (absolutePath.endsWith(QLatin1Char('.'))) {
        int i;
        for (i = absolutePath.count() - 1; i >= 0; --i) {
            if (absolutePath.at(i) != QLatin1Char('.'))
                break;
        }
        absolutePath = absolutePath.left(i+1);
    }
#endif

    QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);
    if ((pathElements.isEmpty() /*|| !QFileInfo(path).exists()*/)   // the path is "/"
#ifndef Q_OS_WIN
        && path != QLatin1String("/")
#endif
        )
    {
        return QModelIndex();
    }
   
    
    pathElements.prepend("/");
    
    return this->find_node_item_by_path_elements(this->tree_root->child_items[0],pathElements,1);
    
    return QModelIndex();
}
QModelIndex RemoteDirModel::find_node_item_by_path_elements( directory_tree_item * parent_node_item ,QStringList & path_elements , int level ) const
{
    if( level < 0 ) return this->createIndex( 0, 0, this->tree_root ) ;
    QString aim_child_node_item_name = path_elements.at(level);
    Q_ASSERT( parent_node_item);
    //qDebug()<<__LINE__<< parent_node_item->file_name<<level<<path_elements ;
    for( int i = 0 ; i < parent_node_item->child_items.size() ; i ++ )
    {
        directory_tree_item * child_item = parent_node_item->child_items[i];
        //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<< child_item->file_name;
        if( child_item->file_name.compare(aim_child_node_item_name) ==0 )
        {
            
            if( level == path_elements.count()-1 ) 
            {
                //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
                QModelIndex mi = this->createIndex( i, 0, child_item );
                //return child_item;
                return mi ;
            }
            else
            {
                //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
                QModelIndex mi =this->find_node_item_by_path_elements( child_item,path_elements,level+1 );
                return mi ; 
            }
        }
    }
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    return this->createIndex( 0, 0, parent_node_item );
    //return parent_node_item ;
}
QModelIndex RemoteDirModel::parent ( const QModelIndex &child ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    if ( !child.isValid() )
    {
        //qDebug()<<" ! child.isValid()";
        return QModelIndex();
    }

    directory_tree_item  *child_item = static_cast< directory_tree_item *> ( child.internalPointer() );
    directory_tree_item  *parent_item = child_item->parent_item;

    if ( !parent_item || parent_item == this->tree_root )
    {
        //qDebug()<<"  !parent_item || parent_item == this->tree_root ";
        return QModelIndex();
    }

    //qDebug()<< " parent_item->row_number ";
    return createIndex ( parent_item->row_number, 0, parent_item );

    return QModelIndex();
}

QVariant RemoteDirModel::data ( const QModelIndex &index, int role ) const
{
    QVariant ret_var ;
    QString unicode_name ;
    char mem[32] = {0};
    struct tm *ltime = 0;

    if( role == Qt::DecorationRole && index.column()==0) {
        if(this->isDir(index))
            return qApp->style()->standardIcon(QStyle::SP_DirIcon);
        else
            return qApp->style()->standardIcon(QStyle::SP_FileIcon);
    }
    
    if ( role != Qt::DisplayRole )
        return QVariant();

    directory_tree_item *item = static_cast< directory_tree_item*> ( index.internalPointer() );

    switch(index.column()) {
    case 0:
        ret_var = item->fileName();
        break;
    case 2:
        strmode(item->attrib.permissions, mem);
        ret_var = QString(mem);
        break;
    case 1:
        ret_var = QString("%1").arg(item->attrib.filesize);
        break;
    case 3:
        ltime = localtime((time_t*)&item->attrib.mtime);
        if (ltime != NULL) {
            strftime(mem, sizeof(mem), "%Y/%m/%d %H:%M:%S", ltime);
        }
        ret_var = QString(mem);
        //ret_var = QVariant ( item->file_date );
        break;
    default:
        return QVariant();
    }
    return ret_var ;
}

Qt::ItemFlags RemoteDirModel::flags ( const QModelIndex &index ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if ( !index.isValid() )
        return Qt::ItemIsEnabled;
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    if(index.column() == 0) {
        return flags | Qt::ItemIsEditable;
    }else{
        return flags;
    }
}
QVariant RemoteDirModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //qDebug() <<"section:" << section ;

    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch ( section ) {
        case 0:
            return QVariant ( tr("Name") );
        case 1:
            return QVariant ( tr("Size") );
        case 2:
            return QVariant ( tr("Type") );
        case 3:
            return QVariant ( tr("Date") );
        default:
            return QVariant ( tr("what are you want?" ));
        }
    }

    return QVariant();

}

int RemoteDirModel::columnCount ( const QModelIndex &/*parent*/ ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    return 4 ;
}

int RemoteDirModel::rowCount(const QModelIndex &parent) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    int row_count = 0;

    if (!parent.isValid()){
        row_count = 1 ;
    }else{
        directory_tree_item * parent_item = static_cast<directory_tree_item*> ( parent.internalPointer() );

        if ( parent_item->retrived == 0
             // || parent_item->retrived == 1
             || parent_item->retrived == 2 ) //为了lazy模式的需要，才做一个假数据。
        {
            //或者是不是要在这里取子结点的行数，即去远程找数据呢。
            //row_count =1 ;
            this->remote_dir_retrive_thread->add_node (parent_item, parent.internalPointer());
            //emit this->sigWantData(0);
            //this->remote_dir_retrive_thread->wantData(0);
        }else{
            row_count = parent_item->child_items.size();
        }
        row_count = parent_item->child_items.size();
    }
    //qDebug()<< "row_count="<<row_count ;
    return row_count ;
}

bool RemoteDirModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
    if(!index.isValid()) {
        return false;
    }
    if(role != Qt::EditRole) return false;
    if(value.toString().length() <= 0) return false;
    assert(index.column() == 0);

    directory_tree_item *parent_item = static_cast<directory_tree_item*>(index.parent().internalPointer());
    directory_tree_item *dti =  static_cast<directory_tree_item*>(index.internalPointer());
    int cmd = SSH2_FXP_RENAME;
    QString data = dti->file_name + "!" + value.toString();

    if(dti->file_name == value.toString()) return false;

    dti->strip_path = parent_item->filePath() + "/" + value.toString();
    dti->file_name = value.toString();//这时是不能修改这个值,否则上句命令执行的时候找不到原文件名
    this->remote_dir_retrive_thread->slot_execute_command(parent_item, parent_item, cmd, data);

    return true;
}

bool RemoteDirModel::insertRows ( int row, int count, const QModelIndex & parent)
{
    q_debug()<<"";
    return true;
}
bool RemoteDirModel::removeRows ( int row, int count, const QModelIndex & parent)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    qDebug() <<" row = "<< row << " count = "<< count;
    if ( parent.isValid() )
        qDebug() << parent;
    else
        qDebug() << parent.isValid()  ;
    directory_tree_item * parent_item = static_cast<directory_tree_item*> ( parent.internalPointer() );
    qDebug() << parent_item ;
    directory_tree_item * delete_item = 0 ;
    directory_tree_item * temp_item  = 0 ;
    
    this->beginRemoveRows ( parent, row, row+count );
    this->dump_tree_node_item ( parent_item );
    
    for( int i = row + count -1 ; i >= row ; i --)
    {
        delete_item = parent_item->child_items[i];
        if ( i !=parent_item->child_items.size()-1 )
        {
            for ( int j = i+1 ; j < parent_item->child_items.size() ; j ++ )
            {
                temp_item = parent_item->child_items[j];
                temp_item->row_number = j-1 ;
                parent_item->child_items[j-1] = temp_item ;
            }
            int c_size = parent_item->child_items.size() ;
            parent_item->child_items.erase ( c_size-1 );
        }
        else
        {
            int c_size = parent_item->child_items.size() ;
            parent_item->child_items.erase ( c_size-1 );    
        }
        delete delete_item ; delete_item = 0 ;
    }
    this->endRemoveRows ();
    return true;
}
bool RemoteDirModel::rowMoveTo ( const QModelIndex & from , const QModelIndex & to )
{
    return true;
}

bool RemoteDirModel::insertColumns ( int column, int count, const QModelIndex & parent /*= QModelIndex() */ )
{
    return true ;
}
bool RemoteDirModel::setItemData ( const QModelIndex & index, const QMap<int, QVariant> & roles )
{
    return true ;
}

void RemoteDirModel::dump_tree_node_item ( directory_tree_item * node_item ) const
{
    directory_tree_item * item = ( directory_tree_item * ) item ;
    assert ( node_item != 0 );
    qDebug() <<"====================>>>>";
    qDebug() <<"name="<<QString ( node_item->file_name );
    qDebug() <<"Type="<<QString ( node_item->file_type );
    qDebug() <<"Size="<<QString ( node_item->file_size );
    qDebug() <<"Date="<<QString ( node_item->file_date );
    qDebug() <<"Retrived="<<node_item->retrived;
    qDebug() <<"prev_retr_flag="<<node_item->prev_retr_flag;
    qDebug() <<"ChildCount="<<node_item->child_items.size();
    qDebug() << "DeleteFlag="<<node_item->delete_flag;
    qDebug() <<"<<<<====================";
}

void RemoteDirModel::slot_remote_dir_node_retrived (
                                                    directory_tree_item*  parent_item,void *  parent_model_internal_pointer )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    int row , col = 0;
    for ( int i = parent_item->child_items.size()-1 ; i >=0  ; i -- )
    {
        if ( parent_item->child_items[i]->delete_flag == 1 )
        {
            row = parent_item->child_items[i]->row_number ;
            qDebug() << "find should delete item "<< i
                     <<" row num:"<< row ;

            this->removeRows ( row,1,
                               this->createIndex ( parent_item->row_number,0
                                                   ,parent_model_internal_pointer ) );
        }
    }

    emit layoutChanged();

    /*
      void columnsAboutToBeInserted ( const QModelIndex & parent, int start, int end )
      void columnsAboutToBeRemoved ( const QModelIndex & parent, int start, int end )
      void columnsInserted ( const QModelIndex & parent, int start, int end )
      void columnsRemoved ( const QModelIndex & parent, int start, int end )
      void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight )
      void headerDataChanged ( Qt::Orientation orientation, int first, int last )
      void layoutAboutToBeChanged ()
      void layoutChanged ()
      void modelAboutToBeReset ()
      void modelReset ()
      void rowsAboutToBeInserted ( const QModelIndex & parent, int start, int end )
      void rowsAboutToBeRemoved ( const QModelIndex & parent, int start, int end )
      void rowsInserted ( const QModelIndex & parent, int start, int end )
      void rowsRemoved ( const QModelIndex & parent, int start, int end ) 
    */
}


Qt::DropActions RemoteDirModel::supportedDropActions () const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList RemoteDirModel::mimeTypes() const
{
    QStringList mtypes;
    mtypes<< "text/uri-list";
    //mtypes << "text/plain";
    mtypes << QAbstractItemModel::mimeTypes();
    //qDebug()<< mtypes ;
    //mtypes = QAbstractItemModel::mimeTypes();
    //qDebug()<< mtypes ;
    //return QAbstractItemModel::mimeTypes();
    return mtypes ;

}
QMimeData *RemoteDirModel::mimeData ( const QModelIndexList &indexes ) const
{
    QMimeData * md = new QMimeData();
    QByteArray encodedData;

    //读取出来indexes中的目录路径信息，封装成特定mine 类型的数据即可以了。
    QString file_name = "file:///hahahha";
    QString file_type ;

    //现在还没有支持多选择，那么我们就只处理一个即可,不用考虑那么多
    directory_tree_item * selected_item = static_cast<directory_tree_item*> ( indexes.at ( 0 ).internalPointer() );

    assert ( selected_item != NULL );

    file_name = selected_item->strip_path  ;
    file_type = selected_item->file_type  ;

    //格式说明：sftp://file_name||file_type
    file_name = QString ( "sftp://" + file_name+"||"+file_type ).toAscii() ;
    encodedData = file_name.toAscii();
    //QList<QUrl> urls ;
    //application/x-qabstractitemmodeldatalist
    md->setData ( "text/uri-list",encodedData );
    //urls.append(QUrl(file_name));

    //md->setUrls(urls);

    return md ;
}
bool RemoteDirModel::dropMimeData ( const QMimeData *data, Qt::DropAction action,
                                    int row, int column, const QModelIndex &parent )
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    qDebug()<<data->urls()<<parent<<sender();
    emit this->sig_drop_mime_data( data, action,row, column , parent );
    bool ret = true ;
    // 	qDebug() <<"signals emited";
    return true ;
    return ret ;
}

void RemoteDirModel::slot_remote_dir_node_clicked ( const QModelIndex & index )
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    directory_tree_item * clicked_item = 0;
    clicked_item = static_cast<directory_tree_item*> ( index.internalPointer() );

    if(index.isValid() == false) return;
    //this->dump_tree_node_item(clicked_item);

    if ( clicked_item->retrived == 1 ) // 半满状态结点
    {
        this->remote_dir_retrive_thread->add_node ( clicked_item,index.internalPointer() );
    }else{
        //no op needed
    }
}

void RemoteDirModel::slot_execute_command( directory_tree_item* parent_item , 
                                           void * parent_model_internal_pointer, int cmd , QString params )
{
    this->remote_dir_retrive_thread->slot_execute_command(parent_item,parent_model_internal_pointer,cmd , params);
}

//TODO
void RemoteDirModel::set_keep_alive(bool keep_alive,int time_out)
{
    //this->keep_alive_interval = time_out ;
    //this->keep_alive = keep_alive ;
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //qDebug()<<keep_alive<<time_out;
    
    if( time_out != this->keep_alive_interval)
    {
        this->keep_alive_interval = time_out  ;
        this->keep_alive_timer->setInterval(this->keep_alive_interval);
    }
    //assert(1==2);
    if( keep_alive != this->keep_alive ) {
        if( keep_alive == false ) {
            this->keep_alive_timer->stop();
        }else{
            this->keep_alive_timer->start();
        }
        this->keep_alive = keep_alive ;
    }
}

void RemoteDirModel::slot_keep_alive_time_out()
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->remote_dir_retrive_thread->slot_execute_command(0,0,SSH2_FXP_KEEP_ALIVE,"");
    
}
QString RemoteDirModel::filePath(const QModelIndex &index) const
{
    directory_tree_item * node_item = 0;
    node_item =  static_cast<directory_tree_item*>(index.internalPointer());
    assert(node_item != 0);

    return node_item->filePath();
}
bool RemoteDirModel::isDir(const QModelIndex &index) const
{
    directory_tree_item * node_item = static_cast<directory_tree_item*>(index.internalPointer()) ;
    assert(node_item != 0);

    return node_item->isDir();
    //depcreated
    /*
    if( node_item->child_items.size() > 0 
        || node_item->file_type.at(0) == QChar('d') 
        || node_item->file_type.at(0) == QChar('D') 
        || node_item->file_type.at(0) == QChar('l') 
        )
        return true ;
    return false ;
    */
}


