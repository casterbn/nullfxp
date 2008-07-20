// remotedirretrivethread.cpp --- 
// 
// Filename: remotedirretrivethread.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 日  5月 25 09:50:50 2008 (CST)
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

#include <vector>
#include <map>
#include <string>

#include <QtCore>

#ifdef WIN32
#include <winsock2.h>
#endif

#include "utils.h"
#include "globaloption.h"
#include "remotedirretrivethread.h"

#include "rfsdirnode.h"


///////////////////////////////////
RemoteDirRetriveThread::RemoteDirRetriveThread (QObject* parent )
    : QThread ( parent )
{
}


RemoteDirRetriveThread::~RemoteDirRetriveThread()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    libssh2_sftp_shutdown(this->ssh2_sftp);
    libssh2_session_disconnect( this->ssh2_sess,"SSH_DISCONNECT_BY_APPLICATION" );
    libssh2_session_free( this->ssh2_sess );
}

void RemoteDirRetriveThread::set_ssh2_handler( void * ssh2_sess )
{
    this->ssh2_sess = (LIBSSH2_SESSION*) ssh2_sess ;
    this->ssh2_sftp = libssh2_sftp_init( this->ssh2_sess ) ;
    assert( this->ssh2_sftp != 0 );
}

void RemoteDirRetriveThread::run()
{
    emit enter_remote_dir_retrive_loop();
    
    int exec_ret_code = -1 ;
    command_queue_elem * cmd_elem;
    while( this->command_queue.size() > 0 )
    {
        cmd_elem = this->command_queue.at(0);
        switch(cmd_elem->cmd)
        {
        case SSH2_FXP_READDIR:
            this->dir_node_process_queue.insert(std::make_pair(cmd_elem->parent_item,cmd_elem->parent_model_internal_pointer) );                
            exec_ret_code = this->retrive_dir();            
            break;
        case SSH2_FXP_MKDIR:
            exec_ret_code = this->mkdir();
            break;
        case SSH2_FXP_RMDIR:
            exec_ret_code = this->rmdir();
            break;
        case SSH2_FXP_REMOVE:
            exec_ret_code = this->rm_file_or_directory_recursively();
            break;
        case SSH2_FXP_RENAME:
            exec_ret_code = this->rename();
            break ;
        case SSH2_FXP_KEEP_ALIVE:
            exec_ret_code = this->keep_alive();
            break ;
        default:    
            qDebug()<<tr("unknown command:")<<cmd_elem->cmd;                
            break;
        }
        //delet item form queue , stopping infinite cycle
        this->command_queue.erase(this->command_queue.begin());
        delete cmd_elem ; cmd_elem = 0 ;
    }

    emit this->leave_remote_dir_retrive_loop();    
}

int  RemoteDirRetriveThread::retrive_dir()
{
    int exec_ret = -1;
    
    directory_tree_item * parent_item, *new_item, *tmp_item;
    void *  parent_model_internal_pointer ;

    QString tmp;
    QVector<directory_tree_item*> deltaItems;
    QVector< QMap< char, QString > > fileinfos;
    char file_name[PATH_MAX+1] ;
    int fxp_ls_ret = 0 ;
    char file_size[20] = {0};
    char file_date[32] = {0};
    char file_type[20] = {0};
 
    while ( this->dir_node_process_queue.size() >0 )
    {
        std::map<directory_tree_item*,void * >::iterator mit;
        mit = this->dir_node_process_queue.begin();

        parent_item = mit->first ;
        parent_model_internal_pointer = mit->second ;
       
        fileinfos.clear();
        //状态初始化
        for(int i = 0; i < parent_item->childCount(); i++) {
            parent_item->childAt(i)->delete_flag = 1;
        }

        LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
        memset(&ssh2_sftp_attrib,0,sizeof(LIBSSH2_SFTP_ATTRIBUTES));
        LIBSSH2_SFTP_HANDLE * ssh2_sftp_handle = 0;
        ssh2_sftp_handle = libssh2_sftp_opendir(this->ssh2_sftp,GlobalOption::instance()->remote_codec->fromUnicode(parent_item->strip_path+ ( "/" )).data());

        // ssh2_sftp_handle == 0 是怎么回事呢？ 返回值 应该是
        // 1 . 这个file_name 是一个链接，但这个链接指向的是一个普通文件而不是目录时libssh2_sftp_opendir返回0 , 而 libssh2_sftp_last_error 返回值为 2 == SSH2_FX_NO_SUCH_FILE
        if( ssh2_sftp_handle == 0 ) {
            qDebug()<<" sftp last error: "<< libssh2_sftp_last_error(this->ssh2_sftp)
                    <<(parent_item->strip_path+ ( "/" ))
                    <<GlobalOption::instance()->remote_codec
                ->fromUnicode(parent_item->strip_path+ ( "/" )).data();
        }
        fxp_ls_ret = 0;
        while(ssh2_sftp_handle != 0 &&
              libssh2_sftp_readdir(ssh2_sftp_handle, file_name, PATH_MAX, &ssh2_sftp_attrib ) > 0)
        {
            QMap<char,QString> thefile ;
            if( strlen ( file_name ) == 1 && file_name[0] == '.' ) continue ;
            if( strlen ( file_name ) == 2 && file_name[0] == '.' && file_name[1] == '.') continue ;
            //不处理隐藏文件? 处理隐藏文件,在这要提供隐藏文件，上层使用过滤代理模型提供显示隐藏文件的功能。
            tmp = QString( GlobalOption::instance()->remote_codec->toUnicode(file_name ));
            if(parent_item->setDeleteFlag(tmp, 0)) {
                if(fxp_ls_ret++ == 0) 
                    printf("Already in list, omited %d", fxp_ls_ret), fxp_ls_ret = fxp_ls_ret | 1<<16;
                else 
                    printf(" %d", fxp_ls_ret<<16>>16);
            }else{
                new_item = new directory_tree_item();
                new_item->parent_item = parent_item;
                new_item->strip_path = parent_item->strip_path + QString("/") + file_name ;
                new_item->file_name = tmp;
                new_item->attrib = ssh2_sftp_attrib;
                new_item->retrived = (new_item->isDir()) ? POP_NO_NEED_NO_DATA : POP_NEWEST;
                deltaItems.append(new_item);
            }
        }  
        if(fxp_ls_ret & (1 << 16))   printf("\n");
        fflush(stdout);
	
        //将多出的记录插入到树中
        for(int i = 0 ;i < deltaItems.count(); i ++) {
            deltaItems.at(i)->row_number = parent_item->childCount();
            parent_item->child_items.insert(std::make_pair(parent_item->childCount(), deltaItems.at(i)));
        }

        deltaItems.clear();
        parent_item->prev_retr_flag = parent_item->retrived ;
        parent_item->retrived = POP_NEWEST ;

        //         //////
        this->dir_node_process_queue.erase(parent_item);
        emit this->remote_dir_node_retrived(parent_item,parent_model_internal_pointer);

        if(ssh2_sftp_handle != 0) //TODO 应该在循环上面检测到为0就continue才对啊。
            libssh2_sftp_closedir(ssh2_sftp_handle);
    }

    return exec_ret ;
}

int  RemoteDirRetriveThread::mkdir()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    command_queue_elem * cmd_item = this->command_queue.at(0);
    
    QString abs_path = cmd_item->parent_item->strip_path + QString("/") + cmd_item->params ;
    
    qDebug()<< "abs  path :"<< abs_path  ;
    
    exec_ret = libssh2_sftp_mkdir( ssh2_sftp , GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data() , 0777 );

    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
}

int  RemoteDirRetriveThread::rmdir()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    command_queue_elem * cmd_item = this->command_queue.at(0);
    
    QString abs_path = cmd_item->parent_item->strip_path + "/" + cmd_item->params ;

    qDebug()<< "abs  path :"<< abs_path  ;
    
    if( sys_dirs.contains( abs_path ) )
    {
        qDebug()<<" rm system directory , this is danger.";
    }
    else
    {
        exec_ret = libssh2_sftp_rmdir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data()  );
    }
    //cmd_item->parent_item->retrived = 2 ;   //让上层视图更新这个结点
    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
}

int  RemoteDirRetriveThread::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    directory_tree_item * child_item = 0 ;
    directory_tree_item * parent_item = 0 ;
    command_queue_elem * cmd_item = this->command_queue.at(0);
    parent_item = cmd_item->parent_item ;
    
    QString abs_path = cmd_item->parent_item->strip_path + "/" +  cmd_item->params ;
    qDebug()<< "abs  path :"<< abs_path  ;
    
    if( sys_dirs.contains(  abs_path) )
    {
        qDebug()<<" rm  system directory recusively , this is danger.";
    }else{
        //找到这个要删除的结点并删除
        for( int  i = 0 ; i < parent_item->child_items.size() ; i ++ )
        {
            child_item = parent_item->child_items[i];
            if( child_item->file_name.compare( cmd_item->params ) == 0 )
            {
                qDebug()<<"found whill remove file:"<<child_item->strip_path;
                this->rm_file_or_directory_recursively_ex(child_item->strip_path);
                break ;
            }
        }
    }

    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
}

//TODO 现在删除隐藏文件或者目录还有问题：即以  .  字符开头的项
int  RemoteDirRetriveThread::rm_file_or_directory_recursively_ex( QString parent_path )  // <==> rm -rf
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
   
    QString abs_path  ;
    //char  remote_path[PATH_MAX+1] = {0};
    //char  strip_path[PATH_MAX+1] = {0};
    //char  file_name[PATH_MAX+1] = {0};
    QVector<QMap<char, QString> > fileinfos ;
    directory_tree_item * child_item = 0 ;
    
    //再次从服务器列出目录，然后处理
    //int lflag = 0 ;    
    //     lflag = LS_LONG_VIEW;
    //     lflag |= LS_SHOW_ALL ;
    
    exec_ret = fxp_do_ls_dir(parent_path + ("/") , fileinfos);
    
    int file_count = fileinfos.size();
    //qDebug()<<" rm ex :" << file_count ;
    
    for( int i = file_count -1 ; i >= 0 ; i --)
    {
        //qDebug()<<" lsed file:"<< fileinfos.at(i)['N'] ;
        if( fileinfos.at(i)['T'].at(0) == 'd')
        {
            if( fileinfos.at(i)['N'].compare(".") == 0 
                || fileinfos.at(i)['N'].compare("..") == 0 )
            {
                //qDebug()<<"do ... ls shown . and .. ???";
                continue ;
            }else{
                this->rm_file_or_directory_recursively_ex( parent_path + ("/") + fileinfos.at(i)['N'] );
            }
        }else if(fileinfos.at(i)['T'].at(0) == 'l' || fileinfos.at(i)['T'].at(0) == '-')
        {
            abs_path = parent_path + "/" + fileinfos.at(i)['N'] ;//+ "/" + child_item->file_name ;
            //strcpy( remote_path , abs_path.toAscii().data() );
            //qDebug()<<QString(tr("Removing %1")).arg( remote_path );
            exec_ret = libssh2_sftp_unlink( ssh2_sftp , GlobalOption::instance()->remote_codec->fromUnicode(abs_path) );
        }else{
            qDebug()<<" unknow file type ,don't know how to remove it";
        }
    }
    
    //删除这个目录
    abs_path = parent_path ;//+ "/" + parent_item->file_name ;
    abs_path = GlobalOption::instance()->remote_codec->fromUnicode(abs_path);
    //qDebug()<<"rmdir: "<< abs_path;
    exec_ret = libssh2_sftp_rmdir(ssh2_sftp, abs_path.toAscii().data());
    if( exec_ret != 0 ) //可能这是一个文件，不是目录，那么使用删除文件的指令
    {
        exec_ret = libssh2_sftp_unlink(ssh2_sftp,GlobalOption::instance()->remote_codec->fromUnicode(abs_path) );
        if( exec_ret != 0 )
        {
            qDebug()<< " count remove file or directory ("<< libssh2_sftp_last_error(ssh2_sftp) <<"): "<< abs_path ;
        }
    }
    
    return exec_ret ;
}

// linux 路径名中不能出现的字符： ! 
int  RemoteDirRetriveThread::rename()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    command_queue_elem * cmd_item = this->command_queue.at(0);
    
    size_t sep_pos = cmd_item->params.indexOf('!');
    
    QString abs_path = cmd_item->parent_item->strip_path + "/" +  cmd_item->params.mid(0,sep_pos) ;
    QString abs_path_rename_to = cmd_item->parent_item->strip_path + "/" + cmd_item->params.mid(sep_pos+1,-1) ;
    
    qDebug()<< "abs  path :"<< abs_path  
            << " abs path rename to ;"<< abs_path_rename_to ;
    
    if( sys_dirs.contains(  abs_path ))
    {
        qDebug()<<" rm system directory , this is danger.";
    }
    else
    {
        exec_ret = libssh2_sftp_rename(ssh2_sftp,
                                       GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data() ,
                                       GlobalOption::instance()->remote_codec->fromUnicode(abs_path_rename_to) );
    }

    this->add_node( cmd_item->parent_item , cmd_item->parent_model_internal_pointer );
    
    return exec_ret ;
}

int RemoteDirRetriveThread::keep_alive()
{
    int exec_ret;
    char full_path [PATH_MAX+1] = {0};
    char strip_path [PATH_MAX+1] = {0};

    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
    
    strcpy(full_path,"/nullfxp_keep_alive_dummy_directory");
    strcpy(strip_path,"/");

    exec_ret = libssh2_sftp_stat(ssh2_sftp,full_path,&ssh2_sftp_attrib);
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<< " stat : "<< exec_ret <<" sftp errno:"<< libssh2_sftp_last_error(ssh2_sftp) ;
    //TODO 在网络失去连接的时候如何向上层类通知，并进行重新连接
    return exec_ret ;
}

int RemoteDirRetriveThread::fxp_do_ls_dir ( QString path , QVector<QMap<char, QString> > & fileinfos      )
{
    LIBSSH2_SFTP_HANDLE * sftp_handle = 0 ;
    LIBSSH2_SFTP_ATTRIBUTES ssh2_sftp_attrib ;
    QMap<char, QString> thefile;
    char file_name[PATH_MAX+1];
    char file_size[PATH_MAX+1];
    char file_type[PATH_MAX+1];
    char file_date[PATH_MAX+1];
    int file_count = 0 ;
    
    sftp_handle = libssh2_sftp_opendir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(path).data() );
    if( sftp_handle == 0 )
    {
        return 0;
    }
    else
    {
        fileinfos.clear();
        memset(&ssh2_sftp_attrib,0,sizeof(LIBSSH2_SFTP_ATTRIBUTES));
        while( libssh2_sftp_readdir( sftp_handle , file_name , PATH_MAX , & ssh2_sftp_attrib ) > 0 )
        {
            if( strlen ( file_name ) == 1 && file_name[0] == '.' ) continue ;
            if( strlen ( file_name ) == 2 && file_name[0] == '.' && file_name[1] == '.') continue ;
            //不处理隐藏文件? 处理隐藏文件
            //if( file_name[0] == '.' ) continue ;
            
            memset(file_size,0,sizeof(file_size )) ;
            snprintf(file_size,sizeof(file_size) , "%llu",ssh2_sftp_attrib.filesize );
            
            struct tm *ltime = localtime((time_t*)&ssh2_sftp_attrib.mtime);
            if (ltime != NULL) 
            {
                if (time(NULL) - ssh2_sftp_attrib.mtime < (365*24*60*60)/2)
                    strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
                else
                    strftime(file_date, sizeof file_date, "%Y/%m/%d %H:%M:%S", ltime);
            }

            strmode(ssh2_sftp_attrib.permissions,file_type );
            //printf(" ls dir : %s %s , date=%s , type=%s \n" , file_name , file_size , file_date , file_type );
            thefile.insert( 'N', GlobalOption::instance()->remote_codec->toUnicode(file_name ) );
            thefile.insert( 'T', file_type );
            thefile.insert( 'S', file_size );
            thefile.insert( 'D',  file_date );  
                      
            fileinfos.push_back(thefile);
            memset(&ssh2_sftp_attrib,0,sizeof(LIBSSH2_SFTP_ATTRIBUTES) );
            thefile.clear();
        }
        libssh2_sftp_closedir(sftp_handle);
        return fileinfos.size();
    }
    
    return 0 ; 
}

void RemoteDirRetriveThread::add_node ( directory_tree_item* parent_item ,void * parent_model_internal_pointer )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	
    parent_item->prev_retr_flag = parent_item->retrived ;
    parent_item->retrived = 8 ;
    command_queue_elem * cmd_elem = new command_queue_elem();
    cmd_elem->parent_item = parent_item;
    cmd_elem->parent_model_internal_pointer = parent_model_internal_pointer;
    cmd_elem->cmd = SSH2_FXP_READDIR;
    this->command_queue.push_back ( cmd_elem );
	
    if (  !this->isRunning()  )
    {
        this->start();
    }

    //TODO 检测重复命令
    //     if ( this->dir_node_process_queue.count ( parent_item ) == 0 )
    // 	{
    //         parent_item->prev_retr_flag = parent_item->retrived ;
    //         parent_item->retrived = 8 ;
    //                 
    //         this->dir_node_process_queue.insert ( std::make_pair ( parent_item,parent_model_internal_pointer ) );
    // 		if ( !this->isRunning() )
    // 		{
    // 			this->start();
    // 		}
    // 	}
    // 	else
    // 	{
    // 		qDebug() <<" this node has already in process queue , omitted";
    // 	}
    //assert ( this->dir_node_process_queue.count ( parent_item ) == 1 );

}

void RemoteDirRetriveThread::slot_execute_command(directory_tree_item* parent_item , 
                                                  void * parent_model_internal_pointer,  int cmd , QString params )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    command_queue_elem * cmd_elem = new command_queue_elem();
    cmd_elem->parent_item = parent_item;
    cmd_elem->parent_model_internal_pointer = parent_model_internal_pointer;
    cmd_elem->cmd = cmd;
    cmd_elem->params = params;
    
    this->command_queue.push_back(cmd_elem);
    
    if ( !this->isRunning() )
    {
        this->start();
    }
        
}

