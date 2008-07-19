/* remotedirretrivethread.h --- 
 * 
 * Filename: remotedirretrivethread.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 日  5月 25 09:50:13 2008 (CST)
 * Version: 
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

#ifndef REMOTEDIRRETRIVETHREAD_H
#define REMOTEDIRRETRIVETHREAD_H

#include <cassert>
#include <vector>
#include <map>

#include <queue>

#include <QtCore>
#include <QThread>

#include "sftp.h"
#include "libssh2.h"
#include "libssh2_sftp.h"

#define SSH2_FXP_KEEP_ALIVE 8888

class RFSDirNode;
class directory_tree_item;


/**
   @author liuguangzhao <gzl@localhost>
*/
class RemoteDirRetriveThread : public QThread
{
    Q_OBJECT;
public:
    RemoteDirRetriveThread(QObject* parent=0);

    ~RemoteDirRetriveThread();
    //在实例初始化后马上调用，否则会导致程序崩溃
    void set_ssh2_handler( void * ssh2_sess );
    
    virtual void run();
        
    void add_node(directory_tree_item* parent_item , void * parent_model_internal_pointer );
    
    void slot_execute_command(directory_tree_item* parent_item , void * parent_model_internal_pointer, int cmd , QString params );

private:
    int  retrive_dir();
    int  mkdir();
    int  rmdir();
    int  rm_file_or_directory_recursively();  // <==> rm -rf
    int  rm_file_or_directory_recursively_ex(QString parent_path);  // <==> rm -rf
    int  rename();
        
    int keep_alive() ;
    int fxp_do_ls_dir ( QString parent_path  , QVector<QMap<char, QString> > & fileinfos  );
signals:
    void enter_remote_dir_retrive_loop();
    void leave_remote_dir_retrive_loop();
        
    void remote_dir_node_retrived(directory_tree_item* parent_item , void * parent_model_internal_pointer );
        
    void execute_command_finished( directory_tree_item* parent_item , void * parent_model_internal_pointer, int cmd ,int status );
        
private:

    std::map<directory_tree_item*,void * >   dir_node_process_queue ;
    class command_queue_elem
    {
    public:
        command_queue_elem()
        {  
            this->parent_item=0;
            this->parent_model_internal_pointer = 0 ;
            this->cmd = -1 ;
            this->retry_times = 0 ;
        }
               
        directory_tree_item* parent_item;
        void * parent_model_internal_pointer;
        int  cmd;
        QString  params;
        int  retry_times ;
    };       
    std::vector<command_queue_elem*>  command_queue;
       
    LIBSSH2_SESSION * ssh2_sess;
    LIBSSH2_SFTP * ssh2_sftp ;

    void subtract_existed_model(directory_tree_item * parent_item , QVector<QMap<char,QString> > & new_items );

signals:
    void sigFoundChildNode(RFSDirNode * parent, QMap<char,QString> fmap);
};

#endif
