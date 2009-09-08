// ftpview.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-05 21:49:36 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "info.h"

#include "globaloption.h"
#include "utils.h"

#include "progressdialog.h"
#include "localview.h"
// #include "remoteview.h"
#include "ftpview.h"
#include "remotedirsortfiltermodel.h"

#include "fileproperties.h"
#include "encryptiondetailfocuslabel.h"
#include "encryptiondetaildialog.h"

#ifndef _MSC_VER
#warning "wrapper lower class, drop this include"
#endif

#include "rfsdirnode.h"

#include "completelineeditdelegate.h"

#include "connection.h"

FTPView::FTPView(QMdiArea *main_mdi_area, LocalView *local_view, QWidget *parent)
    : RemoteView(main_mdi_area, local_view, parent)
{
    // this->remoteview.setupUi(this);
    // this->local_view = local_view;
    // this->main_mdi_area = main_mdi_area;
    // this->setObjectName("rv");
    // ///////
    // this->status_bar = new QStatusBar();
    // this->layout()->addWidget(this->status_bar);
    // this->status_bar->addPermanentWidget(this->enc_label = new EncryptionDetailFocusLabel("ENC", this));
    // QObject::connect(this->enc_label, SIGNAL(mouseDoubleClick()),
    //                  this, SLOT(encryption_focus_label_double_clicked()));
    // HostInfoDetailFocusLabel *hi_label = new HostInfoDetailFocusLabel("HI", this);
    // this->status_bar->addPermanentWidget(hi_label);
    // QObject::connect(hi_label, SIGNAL(mouseDoubleClick()), 
    //                 this, SLOT(host_info_focus_label_double_clicked()));

    // ////////////
    // //     this->remoteview.treeView->setAcceptDrops(false);
    // //     this->remoteview.treeView->setDragEnabled(false);
    // //     this->remoteview.treeView->setDropIndicatorShown(false);
    // //     this->remoteview.treeView->setDragDropMode(QAbstractItemView::NoDragDrop);
    // //this->remoteview.treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    
    // QObject::connect(this->remoteview.treeView, SIGNAL(customContextMenuRequested(const QPoint &)),
    //                  this, SLOT(slot_dir_tree_customContextMenuRequested (const QPoint &)));
    // QObject::connect(this->remoteview.tableView,SIGNAL(customContextMenuRequested(const QPoint &)),
    //                  this, SLOT(slot_dir_tree_customContextMenuRequested (const QPoint & )));
    
    // this->init_popup_context_menu();
    
    // this->in_remote_dir_retrive_loop = false;
    // this->remoteview.tableView->test_use_qt_designer_prompt = 0;
    // CompleteLineEditDelegate *delegate = new CompleteLineEditDelegate();
    // this->remoteview.tableView->setItemDelegate(delegate);
    // this->remoteview.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // this->remoteview.treeView->setItemDelegate(delegate);
    // this->remoteview.treeView->setAnimated(true);
}

void FTPView::init_popup_context_menu()
{
    this->dir_tree_context_menu = new QMenu();
    QAction *action ;
    action  = new QAction(tr("Download"), 0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_new_transfer()));
    
    action = new QAction("", 0);
    action->setSeparator(true);
    this->dir_tree_context_menu->addAction(action);
    
    action = new QAction(tr("Refresh"), 0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_refresh_directory_tree()));
    
    action = new QAction(tr("Properties..."), 0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_show_properties()));
    attr_action = action ;
    
    action = new QAction(tr("Show Hidden"), 0);
    action->setCheckable(true);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(toggled(bool)), this, SLOT(slot_show_hidden(bool)));
    
    action = new QAction("", 0);
    action->setSeparator(true);
    this->dir_tree_context_menu->addAction(action);

    //TODO  CUT, COPY, PASTE, ||set initial directory,||open,open with    
    action = new QAction(tr("Copy &Path"), 0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_copy_path()));

    action = new QAction(tr("Copy &URL"), 0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_copy_url()));
        
    action = new QAction(tr("Create directory..."), 0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_mkdir()));
    
    action = new QAction(tr("Delete directory"), 0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_rmdir()));

    action = new QAction(tr("Rename..."),0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(slot_rename()));
    
    action = new QAction("", 0);
    action->setSeparator(true);
    this->dir_tree_context_menu->addAction(action);
        
    //递归删除目录功能，删除文件的用户按钮
    action = new QAction(tr("Remove recursively !!!"), 0);
    this->dir_tree_context_menu->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(rm_file_or_directory_recursively()));
    
}

FTPView::~FTPView()
{
    // because this line will call in base view class RemoteView
    // this->remoteview.treeView->setModel(0);
    // delete this->remote_dir_model;
}

void FTPView::slot_show_fxp_command_log(bool show)
{
    this->remoteview.listView->setVisible(show);    
}

void FTPView::i_init_dir_view()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    this->remote_dir_model = new RemoteDirModel();
    // this->remote_dir_model->set_ssh2_handler(this->ssh2_sess);
    this->remote_dir_model->setConnection(this->conn);
    
    this->remote_dir_model->set_user_home_path(this->user_home_path);
    this->remote_dir_sort_filter_model = new RemoteDirSortFilterModel();
    this->remote_dir_sort_filter_model->setSourceModel(this->remote_dir_model);
    this->remote_dir_sort_filter_model_ex = new RemoteDirSortFilterModelEX();
    this->remote_dir_sort_filter_model_ex->setSourceModel(this->remote_dir_model);
    
    this->remoteview.treeView->setModel(remote_dir_sort_filter_model_ex);
    // this->remoteview.treeView->setModel(this->remote_dir_model);
    this->remoteview.treeView->setAcceptDrops(true);
    this->remoteview.treeView->setDragEnabled(false);
    this->remoteview.treeView->setDropIndicatorShown(true);
    this->remoteview.treeView->setDragDropMode(QAbstractItemView::DropOnly);

    QObject::connect(this->remote_dir_model,
                     SIGNAL(sig_drop_mime_data(const QMimeData *, Qt::DropAction, int, int, const QModelIndex &)),
                     this, SLOT(slot_drop_mime_data(const QMimeData *, Qt::DropAction, int, int, const QModelIndex &)));
    
    QObject::connect(this->remote_dir_model, SIGNAL(enter_remote_dir_retrive_loop()),
                     this, SLOT(slot_enter_remote_dir_retrive_loop()));
    QObject::connect(this->remote_dir_model, SIGNAL(leave_remote_dir_retrive_loop()),
                     this, SLOT(slot_leave_remote_dir_retrive_loop()));
    
    this->remoteview.treeView->expandAll();
    this->remoteview.treeView->setColumnWidth(0, this->remoteview.treeView->columnWidth(0)*2);
    
    //这里设置为true时，导致这个treeView不能正确显示滚动条了，为什么呢?
    //this->remoteview.treeView->setColumnHidden( 1, false);
    this->remoteview.treeView->setColumnWidth(1, 0);//使用这种方法隐藏看上去就正常了。
    this->remoteview.treeView->setColumnHidden(2, true);
    this->remoteview.treeView->setColumnHidden(3, true);
    
    /////tableView
    this->remoteview.tableView->setModel(this->remote_dir_sort_filter_model);
    this->remoteview.tableView->setRootIndex(this->remote_dir_sort_filter_model->index(this->user_home_path));

    //change row height of table 
    if (this->remote_dir_sort_filter_model->rowCount(this->remote_dir_sort_filter_model->index(this->user_home_path)) > 0) {
        this->table_row_height = this->remoteview.tableView->rowHeight(0)*2/3;
    } else {
        this->table_row_height = 20 ;
    }
    for (int i = 0; i < this->remote_dir_sort_filter_model->rowCount(this->remote_dir_sort_filter_model->index(this->user_home_path)); i ++) {
        this->remoteview.tableView->setRowHeight(i, this->table_row_height);
    }
    this->remoteview.tableView->resizeColumnToContents(0);
    
    /////
    QObject::connect(this->remoteview.treeView, SIGNAL(clicked(const QModelIndex &)),
                     this, SLOT(slot_dir_tree_item_clicked(const QModelIndex &)));
    QObject::connect(this->remoteview.tableView, SIGNAL(doubleClicked(const QModelIndex &)),
                     this, SLOT(slot_dir_file_view_double_clicked(const QModelIndex &)));
    QObject::connect(this->remoteview.tableView, SIGNAL(drag_ready()),
                     this, SLOT(slot_drag_ready()));

    //TODO 连接remoteview.treeView 的drag信号
    
    //显示SSH服务器信息
    // QString ssh_server_version = libssh2_session_get_remote_version(this->ssh2_sess);
    // int ssh_sftp_version = libssh2_sftp_get_version(this->ssh2_sftp);
    // QString status_msg = QString("Ready. (%1  SFTP: V%2)").arg(ssh_server_version).arg(ssh_sftp_version); 
    // this->status_bar->showMessage(status_msg);
}

void FTPView::slot_disconnect_from_remote_host()
{
    this->remoteview.treeView->setModel(0);
    delete this->remote_dir_model;
    this->remote_dir_model = 0;
}

void FTPView::slot_dir_tree_customContextMenuRequested(const QPoint & pos)
{
    this->curr_item_view = static_cast<QAbstractItemView*>(sender());
    QPoint real_pos = this->curr_item_view->mapToGlobal(pos);
    real_pos = QPoint(real_pos.x() + 12, real_pos.y() + 36);
    attr_action->setEnabled(!this->in_remote_dir_retrive_loop);
    this->dir_tree_context_menu->popup(real_pos);
}

void FTPView::slot_new_transfer()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
     
    QString file_path;
    TaskPackage remote_pkg(PROTO_SFTP);
    
    if (this->in_remote_dir_retrive_loop) {
        QMessageBox::warning(this, tr("Notes:"), tr("Retriving remote directory tree,wait a minute please."));
        return ;
    }
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes();
    
    for(int i = 0 ; i < mil.size(); i +=4 ) {
        QModelIndex midx = mil.at(i);
        directory_tree_item *dti = (directory_tree_item*)
            (this->curr_item_view!=this->remoteview.treeView 
             ? this->remote_dir_sort_filter_model->mapToSource(midx).internalPointer() 
             : (this->remote_dir_sort_filter_model_ex->mapToSource(midx ).internalPointer()));
        qDebug()<<dti->file_name<<" "<<" "<<dti->strip_path;
        file_path = dti->strip_path;
        remote_pkg.files<<file_path;
    }

    remote_pkg.host = this->host_name;
    remote_pkg.port = QString("%1").arg(this->port);
    remote_pkg.username = this->user_name;
    remote_pkg.password = this->password;
    remote_pkg.pubkey = this->pubkey;

    this->slot_new_download_requested(remote_pkg);
}

QString FTPView::get_selected_directory()
{
    QString file_path;
    
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if (ism == 0) {
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));                
        return file_path;
    }
    
    QModelIndexList mil = ism->selectedIndexes();

    for (int i = 0 ; i < mil.size(); i +=4) {
        QModelIndex midx = mil.at(i);
        QModelIndex aim_midx =  this->remote_dir_sort_filter_model_ex->mapToSource(midx);
        directory_tree_item *dti = (directory_tree_item*) aim_midx.internalPointer();
        qDebug()<<dti->file_name <<" "<< dti->strip_path;
        if (this->remote_dir_sort_filter_model_ex->isDir(midx)) {
        	  file_path = dti->strip_path;
        } else {
        	  file_path = "";
        }
    }
    
    return file_path;
}

// void FTPView::set_ssh2_handler(void *ssh2_sess, int ssh2_sock)
// {
//     this->ssh2_sess = (LIBSSH2_SESSION*)ssh2_sess ;
//     this->ssh2_sftp = libssh2_sftp_init(this->ssh2_sess);
//     assert(this->ssh2_sftp != 0);
    
//     this->ssh2_sock = ssh2_sock;
// }

// void FTPView::set_host_info(QString host_name, QString user_name, QString password, short port, QString pubkey)
// {

//     this->host_name = host_name ;
//     this->user_name = user_name ;
//     this->password = password ;
//     this->port = port;
//     this->pubkey = pubkey ;

//     this->setWindowTitle(this->windowTitle() + ": " + this->user_name + "@" + this->host_name);
// }

// void FTPView::set_user_home_path(std::string user_home_path)
// {
//     this->user_home_path = user_home_path ;
// }

void FTPView::setConnection(Connection *conn)
{
    this->conn = conn;
    this->setWindowTitle(this->conn->protocol + ": " + this->conn->userName + "@" + this->conn->hostName);
}

void FTPView::closeEvent(QCloseEvent *event)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    event->ignore();
    if (this->in_remote_dir_retrive_loop) {
        //TODO 怎么能友好的结束
        //QMessageBox::warning(this,tr("Attentions:"),tr("Retriving remote directory tree, wait a minute please.") );
        //return ;
        //如果说是在上传或者下载,则强烈建议用户先关闭传输窗口，再关闭连接
        if (this->own_progress_dialog != 0) {
            QMessageBox::warning(this, tr("Attentions:"), tr("You can't close connection when transfering file."));
            return ;
        }
    }
    //this->setVisible(false);
    if (QMessageBox::question(this, tr("Attemp to close this window?"),tr("Are you sure disconnect from %1?").arg(this->windowTitle()), QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok) {
        this->setVisible(false);
        qDebug()<<"delete remote view";
        delete this ;
    }
}
void FTPView::slot_custom_ui_area()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QSizePolicy sp;
    sp.setVerticalPolicy(QSizePolicy::Ignored);
    this->remoteview.listView->setSizePolicy( sp ) ;
    //这个设置必须在show之前设置才有效果
    this->remoteview.splitter->setStretchFactor(0,1);
    this->remoteview.splitter->setStretchFactor(1,2);

    this->remoteview.splitter_2->setStretchFactor(0,6);
    this->remoteview.splitter_2->setStretchFactor(1,1);
    this->remoteview.listView->setVisible(false);//暂时没有功能在里面先隐藏掉
    //this->remoteview.tableView->setVisible(false);
    qDebug()<<this->geometry();
    this->setGeometry(this->x(),this->y(),this->width(),this->height()*2);
    qDebug()<<this->geometry();
}

void FTPView::slot_enter_remote_dir_retrive_loop()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->in_remote_dir_retrive_loop = true ;
    this->remote_dir_model->set_keep_alive(false);
    this->orginal_cursor = this->remoteview.splitter->cursor();
    this->remoteview.splitter->setCursor(Qt::BusyCursor);
}

void FTPView::slot_leave_remote_dir_retrive_loop()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    this->remoteview.splitter->setCursor(this->orginal_cursor);
    this->remote_dir_model->set_keep_alive(true);
    this->in_remote_dir_retrive_loop = false ;
    for (int i = 0 ; i < this->remote_dir_sort_filter_model->rowCount(this->remoteview.tableView->rootIndex()); i ++) {
        this->remoteview.tableView->setRowHeight(i, this->table_row_height);
    }
    this->remoteview.tableView->resizeColumnToContents ( 0 );
}

void FTPView::update_layout()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QString file_path ;
    
    QItemSelectionModel *ism = this->remoteview.treeView->selectionModel();
    
    if (ism == 0) {
        //QMessageBox::critical(this,tr("waring..."),tr("maybe you haven't connected"));                
        //return file_path ;
        qDebug()<<" why???? no QItemSelectionModel??";        
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes();
    
    if (mil.count() == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
    }
    
    for (int i = 0 ; i < mil.size(); i += 4) {
        QModelIndex midx = mil.at(i);
        qDebug()<<midx ;
        //这个地方为什么不使用mapToSource会崩溃呢？
        directory_tree_item *dti = static_cast<directory_tree_item*>
            (this->remote_dir_sort_filter_model_ex->mapToSource(midx).internalPointer());
        qDebug()<<dti->file_name<<" "<< dti->strip_path;
        file_path = dti->strip_path;
        dti->retrived = 1;
        dti->prev_retr_flag=9;
        this->remote_dir_model->slot_remote_dir_node_clicked(this->remote_dir_sort_filter_model_ex->mapToSource(midx));
    }
}

void FTPView::slot_refresh_directory_tree()
{
    this->update_layout();
}

void FTPView::slot_show_properties()
{
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";        
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes();
    QModelIndexList aim_mil ;
    if (this->curr_item_view == this->remoteview.treeView) {
        for (int i = 0 ; i < mil.count() ; i ++) {
            aim_mil << this->remote_dir_sort_filter_model_ex->mapToSource(mil.at(i));
        }
    } else {
        for (int i = 0 ; i < mil.count() ; i ++) {
            aim_mil << this->remote_dir_sort_filter_model->mapToSource(mil.at(i));
        }
    }
    if (aim_mil.count() == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";
        return;
    }
    //  文件类型，大小，几个时间，文件权限
    //TODO 从模型中取到这些数据并显示在属性对话框中。
    FileProperties * fp = new FileProperties(this);
    fp->set_ssh2_sftp(this->ssh2_sftp);
    fp->set_file_info_model_list(aim_mil);
    fp->exec();
    delete fp ;
}

void FTPView::slot_mkdir()
{
    QString dir_name ;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));                
        return ;
    }
    
    QModelIndexList mil = ism->selectedIndexes();
    
    if (mil.count() == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
        return ;
    }
    
    QModelIndex midx = mil.at(0);
    QModelIndex aim_midx = (this->curr_item_view == this->remoteview.treeView) ? this->remote_dir_sort_filter_model_ex->mapToSource(midx): this->remote_dir_sort_filter_model->mapToSource(midx) ;
    directory_tree_item *dti = (directory_tree_item*)(aim_midx.internalPointer());
    
    //检查所选择的项是不是目录
    if (!this->remote_dir_model->isDir(aim_midx)) {
        QMessageBox::critical(this, tr("waring..."), tr("The selected item is not a directory."));
        return ;
    }
    
    dir_name = QInputDialog::getText(this, tr("Create directory:"),
                                     tr("Input directory name:")
                                     +"                                                        ",
                                     QLineEdit::Normal,
                                     tr("new_direcotry"));
    if (dir_name == QString::null) {
        return ;
    } 
    if (dir_name.length() == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("waring..."), tr("no directory name supplyed "));
        return;
    }
    //TODO 将 file_path 转换编码再执行下面的操作
    
    this->remote_dir_model->slot_execute_command(dti, aim_midx.internalPointer(), SSH2_FXP_MKDIR, dir_name);
    
}

void FTPView::slot_rmdir()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));                
        return  ;
    }
    
    QModelIndexList mil = ism->selectedIndexes();
    
    if (mil.count() == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected")+"                         ");
        return ;
    }
    
    QModelIndex midx = mil.at(0);
    QModelIndex aim_midx = (this->curr_item_view == this->remoteview.treeView) 
        ? this->remote_dir_sort_filter_model_ex->mapToSource(midx)
        : this->remote_dir_sort_filter_model->mapToSource(midx);    
    directory_tree_item *dti = (directory_tree_item*) aim_midx.internalPointer();
    QModelIndex parent_model =  aim_midx.parent() ;
    directory_tree_item *parent_item = (directory_tree_item*)parent_model.internalPointer();
    
    //TODO 检查所选择的项是不是目录
    
    this->remote_dir_model->slot_execute_command(parent_item, parent_model.internalPointer(),
                                                 SSH2_FXP_RMDIR, dti->file_name   );
    
}

void FTPView::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));                
        return  ;
    }
    
    QModelIndexList mil = ism->selectedIndexes();
    
    if (mil.count() == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected"));
        return ;
    }
    //TODO 处理多选的情况
    QModelIndex midx = mil.at(0);
    QModelIndex aim_midx = (this->curr_item_view == this->remoteview.treeView) 
        ? this->remote_dir_sort_filter_model_ex->mapToSource(midx)
        : this->remote_dir_sort_filter_model->mapToSource(midx);
    directory_tree_item *dti = (directory_tree_item*) aim_midx.internalPointer();
    if (QMessageBox::warning(this, tr("Warning:"), 
                            tr("Are you sure remove this directory and it's subnodes"),
                            QMessageBox::Yes, QMessageBox::Cancel) == QMessageBox::Yes) {
        QModelIndex parent_model =  aim_midx.parent() ;
        directory_tree_item *parent_item = (directory_tree_item*)parent_model.internalPointer();
        
        this->remote_dir_model->slot_execute_command(parent_item, parent_model.internalPointer(),
                                                     SSH2_FXP_REMOVE, dti->file_name);
    }
}

void FTPView::slot_rename()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("waring..."), tr("maybe you haven't connected"));                
        return  ;
    }
    
    QModelIndexList mil = ism->selectedIndexes();
    
    if (mil.count() == 0 ) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("waring..."), tr("no item selected"));
        return ;
    }
    this->curr_item_view->edit(mil.at(0));
}
void FTPView::slot_copy_path()
{
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));
        return  ;
    }
    
    QModelIndexList mil = ism->selectedIndexes();
    
    if (mil.count() == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected")+"                         ");
        return ;
    }
    
    QModelIndex midx = mil.at(0);
    QModelIndex aim_midx = (this->curr_item_view == this->remoteview.treeView) 
        ? this->remote_dir_sort_filter_model_ex->mapToSource(midx)
        : this->remote_dir_sort_filter_model->mapToSource(midx);    
    directory_tree_item *dti = (directory_tree_item*)aim_midx.internalPointer();

    QApplication::clipboard()->setText(dti->strip_path);
}

void FTPView::slot_copy_url()
{
    QItemSelectionModel *ism = this->curr_item_view->selectionModel();
    
    if (ism == 0) {
        qDebug()<<" why???? no QItemSelectionModel??";
        QMessageBox::critical(this, tr("Waring..."), tr("Maybe you haven't connected"));
        return  ;
    }
    
    QModelIndexList mil = ism->selectedIndexes();
    
    if (mil.count() == 0) {
        qDebug()<<" selectedIndexes count :"<< mil.count() << " why no item selected????";
        QMessageBox::critical(this, tr("Waring..."), tr("No item selected")+"                         ");
        return ;
    }
    
    QModelIndex midx = mil.at(0);
    QModelIndex aim_midx = (this->curr_item_view == this->remoteview.treeView) 
        ? this->remote_dir_sort_filter_model_ex->mapToSource(midx)
        : this->remote_dir_sort_filter_model->mapToSource(midx);    
    directory_tree_item *dti = (directory_tree_item*) aim_midx.internalPointer();

    QString url = QString("sftp://%1@%2:%3%4").arg(this->user_name)
        .arg(this->host_name).arg(this->port).arg(dti->strip_path);
    QApplication::clipboard()->setText(url);
}


void FTPView::slot_new_upload_requested(TaskPackage local_pkg, TaskPackage remote_pkg)
{
    FTPView *remote_view = this ;
    ProgressDialog *pdlg = new ProgressDialog();

    pdlg->set_transfer_info(local_pkg, remote_pkg);

    QObject::connect(pdlg, SIGNAL(transfer_finished(int, QString)),
                     remote_view, SLOT(slot_transfer_finished (int, QString)));

    this->main_mdi_area->addSubWindow(pdlg);
    pdlg->show();
    this->own_progress_dialog = pdlg;

}
void FTPView::slot_new_upload_requested(TaskPackage local_pkg)
{
    QString remote_file_name ;
    FTPView *remote_view = this ;
    TaskPackage remote_pkg(PROTO_SFTP);

    qDebug()<<" window title :" << remote_view->windowTitle();

    remote_file_name = remote_view->get_selected_directory();    

    if (remote_file_name.length() == 0) {
        QMessageBox::critical(this, tr("Waring..."), tr("you should selecte a remote file directory."));
    } else {
        remote_pkg.files<<remote_file_name;
        remote_pkg.host = this->host_name;
        remote_pkg.username = this->user_name;
        remote_pkg.password = this->password;
        remote_pkg.port = QString("%1").arg(this->port);
        remote_pkg.pubkey = this->pubkey;

        this->slot_new_upload_requested(local_pkg, remote_pkg);
    }
}

void FTPView::slot_new_download_requested(TaskPackage local_pkg, TaskPackage remote_pkg)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    FTPView *remote_view = this ;
        
    ProgressDialog *pdlg = new ProgressDialog(0);
    // src is remote file , dest if localfile 
    pdlg->set_transfer_info(remote_pkg, local_pkg);
    QObject::connect(pdlg, SIGNAL(transfer_finished(int, QString)),
                     this, SLOT(slot_transfer_finished(int, QString)));
    this->main_mdi_area->addSubWindow(pdlg);
    pdlg->show();
    this->own_progress_dialog = pdlg;
}
void FTPView::slot_new_download_requested(TaskPackage remote_pkg) 
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QString local_file_path;
    FTPView *remote_view = this;
    TaskPackage local_pkg(PROTO_FILE);
    
    local_file_path = this->local_view->get_selected_directory();
    
    qDebug()<<local_file_path;
    if (local_file_path.length() == 0 
        || !QFileInfo(local_file_path).isDir()) {
        //        || !is_dir(GlobalOption::instance()->locale_codec->fromUnicode(local_file_path).data())) {
        qDebug()<<" selected a local file directory  please";
        QMessageBox::critical(this, tr("waring..."), tr("you should selecte a local file directory."));
    } else {
        local_pkg.files<<local_file_path;
        this->slot_new_download_requested(local_pkg, remote_pkg);
    }
}

void FTPView::slot_transfer_finished(int status, QString errorString)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    FTPView *remote_view = this ;
    
    ProgressDialog *pdlg = (ProgressDialog*)sender();

    if (status == 0 || status ==3) {
        //TODO 通知UI更新目录结构,在某些情况下会导致左侧树目录变空。
        //int transfer_type = pdlg->get_transfer_type();
        //if ( transfer_type == TransferThread::TRANSFER_GET )
        {
            this->local_view->update_layout();
        }
        //else if ( transfer_type == TransferThread::TRANSFER_PUT )
        {
            remote_view->update_layout();
        }
        //else
        {
            // xxxxx: 没有预期到的错误
            //assert ( 1== 2 );
        }
    } else if (status != 0 && status != 3) {
        QString errmsg = QString(errorString).arg(status);
        if (errmsg.length() < 50) errmsg = errmsg.leftJustified(50);
        QMessageBox::critical(this, QString(tr("Error: ")), errmsg);
    }
    this->main_mdi_area->removeSubWindow(pdlg->parentWidget());

    delete pdlg;
    this->own_progress_dialog = 0;
    //     remote_view->slot_leave_remote_dir_retrive_loop();
}

/**
 *
 * index 是proxy的index 
 */
void FTPView::slot_dir_tree_item_clicked(const QModelIndex & index)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QString file_path;

    remote_dir_model->slot_remote_dir_node_clicked(this->remote_dir_sort_filter_model_ex->mapToSource(index));
    
    file_path = this->remote_dir_sort_filter_model_ex->filePath(index);
    this->remoteview.tableView->setRootIndex(this->remote_dir_sort_filter_model->index(file_path));
    for (int i = 0 ; i < this->remote_dir_sort_filter_model->rowCount(this->remote_dir_sort_filter_model->index(file_path)); i ++ ) {
        this->remoteview.tableView->setRowHeight(i, this->table_row_height);
    }
    this->remoteview.tableView->resizeColumnToContents(0);
}

void FTPView::slot_dir_file_view_double_clicked(const QModelIndex & index)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //TODO if the clicked item is direcotry ,
    //expand left tree dir and update right table view
    // got the file path , tell tree ' model , then expand it
    //文件列表中的双击事件
    //1。　本地主机，如果是目录，则打开这个目录，如果是文件，则使用本机的程序打开这个文件
    //2。对于远程主机，　如果是目录，则打开这个目录，如果是文件，则提示是否要下载它(或者也可以直接打开这个文件）。
    QString file_path;
    if (this->remote_dir_sort_filter_model->isDir(index)) {
        this->remoteview.treeView->expand(this->remote_dir_sort_filter_model_ex->index(this->remote_dir_sort_filter_model->filePath(index)).parent());
        this->remoteview.treeView->expand(this->remote_dir_sort_filter_model_ex->index(this->remote_dir_sort_filter_model->filePath(index)));
        this->slot_dir_tree_item_clicked(this->remote_dir_sort_filter_model_ex->index(this->remote_dir_sort_filter_model->filePath(index)));
        this->remoteview.treeView->selectionModel()->clearSelection();
        this->remoteview.treeView->selectionModel()->select(this->remote_dir_sort_filter_model_ex->index(this->remote_dir_sort_filter_model->filePath(index)) , QItemSelectionModel::Select);
    } else if (this->remote_dir_sort_filter_model->isSymLink(index)) {
        QModelIndex idx = this->remote_dir_sort_filter_model->mapToSource(index);
        directory_tree_item *node_item = (directory_tree_item*)idx.internalPointer();
        q_debug()<<node_item->strip_path;
        this->remote_dir_model->slot_execute_command(node_item, idx.internalPointer(),
                                                     SSH2_FXP_REALPATH, QString(""));
    } else {
        q_debug()<<"double clicked a regular file, no op now, only now";
    }
}

void FTPView::slot_drag_ready()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //TODO 处理从树目录及文件列表视图中的情况
    //QAbstractItemView * sender_view = qobject_cast<QAbstractItemView*>(sender());
    QString  temp_file_path, remote_file_name;
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    
    //这个视图中所选择的目录优先，如果没有则查找左侧目录树是是否有选择的目录，如果再找不到，则使用右侧表视图的根
    QItemSelectionModel *ism = this->remoteview.tableView->selectionModel();
    QModelIndexList mil = ism->selectedIndexes();
    if (mil.count() == 0) {
        ism = this->remoteview.treeView->selectionModel();
        mil = ism->selectedIndexes();
    }

    TaskPackage tpkg(PROTO_SFTP);
    
    tpkg.host = this->host_name;
    tpkg.username = this->user_name;
    tpkg.password = this->password;
    tpkg.port = QString("%1").arg(this->port);
    tpkg.pubkey = this->pubkey;    

    for (int i = 0 ; i< mil.count() ;i += this->remote_dir_model->columnCount()) {
        QModelIndex midx = mil.at(i);
        temp_file_path = (qobject_cast<RemoteDirModel*>(this->remote_dir_model))
            ->filePath(this->remote_dir_sort_filter_model->mapToSource(midx) );
        tpkg.files<<temp_file_path;
    }
    
    mimeData->setData("application/task-package", tpkg.toRawData());
    drag->setMimeData(mimeData);
    
    if (mil.count() > 0) {
        Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
        Q_UNUSED(dropAction);
    }
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<"drag->exec returned";
}

bool FTPView::slot_drop_mime_data(const QMimeData *data, Qt::DropAction action,
                                     int row, int column, const QModelIndex &parent)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    Q_UNUSED(row);
    Q_UNUSED(column);
    
    TaskPackage local_pkg(PROTO_FILE);
    TaskPackage remote_pkg(PROTO_SFTP);
   
    directory_tree_item *aim_item = static_cast<directory_tree_item*>(parent.internalPointer());        
    QString remote_file_name = aim_item->strip_path ;

    remote_pkg.files<<remote_file_name;
    remote_pkg.host = this->host_name;
    remote_pkg.username = this->user_name;
    remote_pkg.password = this->password;
    remote_pkg.port = QString("%1").arg(this->port);
    remote_pkg.pubkey = this->pubkey;    
    
    if (data->hasFormat("application/task-package")) {
        local_pkg = TaskPackage::fromRawData(data->data("application/task-package"));
        if (local_pkg.isValid(local_pkg)) {
            // TODO 两个sftp服务器间拖放的情况
            this->slot_new_upload_requested(local_pkg, remote_pkg);
        }
    } else if (data->hasFormat("text/uri-list")) {
        // from localview
        QList<QUrl> files = data->urls();
        if (files.count() == 0) {
            // return false;
        } else {
            for (int i = 0 ; i < files.count(); i++) {
                local_pkg.files<<files.at(i).path();
            }
            this->slot_new_upload_requested(local_pkg, remote_pkg);
        }        
    } else {
        qDebug()<<"invalid mime type:"<<data->formats();
    }
    qDebug()<<"drop mime data processed";
    
    return true;
}

void FTPView::slot_show_hidden(bool show)
{
    if (show) {
        remote_dir_sort_filter_model->setFilter(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
        remote_dir_sort_filter_model_ex->setFilter(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
    } else {
        remote_dir_sort_filter_model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        remote_dir_sort_filter_model_ex->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    }
}

void FTPView::encryption_focus_label_double_clicked()
{
    //qDebug()<<__FILE__<<":"<<__LINE__;
    EncryptionDetailDialog *enc_dlg = 0;
    char **server_info, **pptr;
    int sftp_version;

    // pptr = server_info = libssh2_session_get_remote_info(this->ssh2_sess);
    // sftp_version = libssh2_sftp_get_version(this->ssh2_sftp); 

    // enc_dlg = new EncryptionDetailDialog(server_info, this);
    // enc_dlg->exec();

    // //if(server_info != NULL) free(server_info);
    // delete enc_dlg;

    // while (*pptr != NULL) {
    //     free(*pptr);
    //     pptr ++;
    // }
    // free(server_info);
}

void FTPView::host_info_focus_label_double_clicked()
{
    HostInfoDetailFocusLabel *hi_label = (HostInfoDetailFocusLabel*)sender();
    qDebug()<<"hehe"<<hi_label;

    LIBSSH2_CHANNEL *ssh2_channel = 0;
    int rv = -1;
    char buff[1024] ;
    QString evn_output;
    QString uname_output;
    const char *cmd = "uname -a";

    // ssh2_channel = libssh2_channel_open_session((LIBSSH2_SESSION*)this->ssh2_sess);
    // //libssh2_channel_set_blocking(ssh2_channel, 1);
    // rv = libssh2_channel_exec(ssh2_channel, cmd);
    // qDebug()<<"SSH2 exec: "<<rv;
  
    // memset(buff, 0, sizeof(buff));
    // while ((rv = libssh2_channel_read(ssh2_channel, buff, 1000)) > 0) {
    //     qDebug()<<"Channel read: "<<rv<<" -->"<<buff;
    //     uname_output += QString(buff);
    //     memset(buff, 0, sizeof(buff));
    // }

    // libssh2_channel_close(ssh2_channel);
    // libssh2_channel_free(ssh2_channel);
    
    // qDebug()<<"Host Info: "<<uname_output;
    // hi_label->setToolTip(uname_output);
    
    // QDialog *dlg = new QDialog(this);
    // dlg->setFixedWidth(400);
    // dlg->setFixedHeight(100);
    // QLabel * label = new QLabel("", dlg);
    // label->setWordWrap(true);
    // label->setText(uname_output);
    // // dlg->layout()->addWidget(label);
    // dlg->exec();
    // delete dlg;
}

