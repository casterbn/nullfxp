// synchronizewindow.cpp --- 
// 
// Filename: synchronizewindow.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 五  8月  8 13:44:42 2008 (CST)
// Version: 
// Last-Updated: 日  8月 17 13:29:08 2008 (CST)
//           By: 刘光照<liuguangzhao@users.sf.net>
//     Update #: 2
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

#include "utils.h"
#include "remotehostconnectthread.h"
#include "basestorage.h"
#include "sshfileinfo.h"

#include "syncdiffermodel.h"
#include "synchronizewindow.h"

SyncWalker::SyncWalker(QObject *parent)
    :QThread(0)
{
    this->parent = static_cast<SynchronizeWindow*>(parent);
    ssh2_sess = NULL;
    ssh2_sftp = NULL;
    hsftp = NULL;
}
SyncWalker::~SyncWalker()
{
    q_debug()<<"destructured";
}

bool SyncWalker::checkLocalInfo()
{
    this->localBasePath = this->parent->local_dir;
    return true;
    return false;
}

bool SyncWalker::connectToRemoteHost()
{
    this->remoteBasePath = this->parent->remote_dir;

    QMap<QString, QString> host;
    BaseStorage *storage = BaseStorage::instance();
    storage->open();
    this->remoteHost = host = storage->getHost(this->parent->sess_name);
    q_debug()<<host;
    
    RemoteHostConnectThread *conn 
        = new RemoteHostConnectThread(host["user_name"], host["password"], host["host_name"],
                                      host["port"].toShort(), host["pubkey"]);
    conn->run();
    ssh2_sess = (LIBSSH2_SESSION*)conn->get_ssh2_sess();
    delete conn; conn = NULL;
    Q_ASSERT(ssh2_sess != NULL);

    ssh2_sftp = libssh2_sftp_init(ssh2_sess);
    
    if (ssh2_sftp != NULL) {
        Q_ASSERT(ssh2_sftp != NULL);
        q_debug()<<"connect ok";
        return true;
    }
    
    return false;
}
bool SyncWalker::checkRemoteInfo()
{
    Q_ASSERT(ssh2_sess != NULL);
    Q_ASSERT(ssh2_sftp != NULL);

    LIBSSH2_SFTP_ATTRIBUTES attr;
    int rc = 0;
    unsigned long sftp_error = 8888;
    char *sess_errmsg = NULL;
    int  errmsg_len = 0;

    rc = libssh2_sftp_stat(ssh2_sftp, this->remoteBasePath.toAscii().data(), &attr);
    if (rc != 0) {
        sftp_error = libssh2_sftp_last_error(ssh2_sftp);
        rc = libssh2_session_last_error(ssh2_sess, &sess_errmsg, &errmsg_len, 0);
        q_debug()<<"STAT ERROR: "<<rc<<QString(sess_errmsg);
        return false;
    }
    // is dir
    // what link which link to a dir?
    if (! S_ISDIR(attr.permissions)) {
        q_debug()<<"not directory";
        return false;
    }

    // TODO
    // is readable    

    // is writable
    
    q_debug()<<"check info ok";
    return true;
}
bool SyncWalker::disconnectFromRemoteHost()
{
    if (ssh2_sftp != NULL) {
        libssh2_sftp_shutdown(ssh2_sftp);
    }
    if (ssh2_sess != NULL) {
        libssh2_session_free(ssh2_sess);
    }

    return false;
}

QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > 
SyncWalker::getRemoteFiles()
{
    q_debug()<<"";

    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > files;
    LIBSSH2_SFTP_ATTRIBUTES *attr ;
    LIBSSH2_SFTP_HANDLE *hsftp = NULL;

    int rc = 0;
    char fnbuf[512] = {0};
    QString currDir;
    QString currFile;
    QString nopFile;
    QStack<QString> dirStack;
    
    dirStack.push(this->remoteBasePath);

    while (!dirStack.empty()) {
        currDir = dirStack.pop();
        hsftp = libssh2_sftp_opendir(ssh2_sftp, currDir.toAscii().data());
        if (hsftp == NULL) {
            q_debug()<<"opendir error:"<<currDir;
            continue;
        }
        while (true) {
            rc = libssh2_sftp_readdir(hsftp, fnbuf, sizeof(fnbuf), &ssh2_attr);
            if (rc <= 0) {
                q_debug()<<"readdir error:"<<currDir;
                break;
            }
            if ((rc == 1 && fnbuf[0] == '.') || (rc == 2 && fnbuf[0] == '.' && fnbuf[1] == '.')) {
                continue;
            }
            fnbuf[rc] = '\0';
            currFile = QString(fnbuf);
            if (S_ISDIR(ssh2_attr.permissions)) {
                dirStack.push(this->remoteBasePath + "/" + currFile);
            }
            attr = (LIBSSH2_SFTP_ATTRIBUTES*)malloc(sizeof(LIBSSH2_SFTP_ATTRIBUTES));
            attr->flags |= FLAG_REMOTE_FILE;
            memcpy(attr, &ssh2_attr, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
            nopFile = currDir + "/" + currFile;
            nopFile = nopFile.right(nopFile.length() - this->remoteBasePath.length() - 1);
            files.append(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*>(nopFile, attr));
        }
        libssh2_sftp_closedir(hsftp);
    }
    q_debug()<<"return ";
    return files;
}

LIBSSH2_SFTP_ATTRIBUTES *SyncWalker::QFileInfoToLIBSSH2Attribute(QFileInfo &fi)
{
    LIBSSH2_SFTP_ATTRIBUTES *attr = NULL ;

    attr = (LIBSSH2_SFTP_ATTRIBUTES*)malloc(sizeof(LIBSSH2_SFTP_ATTRIBUTES));
    memset(attr, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));

    attr->filesize = fi.size();
    attr->atime = fi.lastRead().toTime_t();
    attr->mtime = fi.lastModified().toTime_t();
    attr->permissions = fi.permissions();
    attr->uid = fi.ownerId();
    attr->gid = fi.groupId();

    attr->flags = LIBSSH2_SFTP_ATTR_SIZE | LIBSSH2_SFTP_ATTR_UIDGID
        | LIBSSH2_SFTP_ATTR_PERMISSIONS | LIBSSH2_SFTP_ATTR_ACMODTIME ;

    return attr ;
}

QFileInfo SyncWalker::LIBSSH2AttributeToQFileInfo(LIBSSH2_SFTP_ATTRIBUTES *attr)
{
    QFileInfo fi;
    Q_UNUSED(attr);
    Q_ASSERT(1 == 2);
    return fi;
}

QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > 
SyncWalker::getLocalFiles()
{
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > files;
    LIBSSH2_SFTP_ATTRIBUTES *attr ;
    QString currDir;
    QString currFile;
    QString nopFile;
    QStack<QString> dirStack;
    
    dirStack.push(this->localBasePath);

    while (!dirStack.empty()) {
        currDir = dirStack.pop();
        QDir dh(currDir);
        QStringList fileList = dh.entryList();
        for(int i = 0 ; i < fileList.count(); i++) {
            if(fileList.at(i) == "." || fileList.at(i) == "..") {
                continue;
            }
            currFile = currDir + "/" + fileList.at(i);
            QFileInfo fi(currFile);
            if (fi.isDir()) {
                dirStack.push(currFile);
            }
            attr = this->QFileInfoToLIBSSH2Attribute(fi);
            Q_ASSERT(attr != NULL);
            attr->flags |= FLAG_LOCAL_FILE;
            nopFile = currFile.right(currFile.length() - this->localBasePath.length() - 1);
            files.append(QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*>(nopFile, attr));
        }
    }
    q_debug()<<"done";
    return files;
}
bool SyncWalker::sameFileTime(unsigned long a, unsigned long b)
{
    if (a < b)
        return b - a <= FILE_TIME_PRECISION;
    else
        return a - b <= FILE_TIME_PRECISION;    
}

QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > 
SyncWalker::sortMerge(
          QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &rfiles,
          QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &lfiles)
{
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > files;
    QString rfile;
    QString lfile;
    LIBSSH2_SFTP_ATTRIBUTES *rattr;
    LIBSSH2_SFTP_ATTRIBUTES *lattr;
    int rcnt = rfiles.count();
    int lcnt = lfiles.count();
    int  found;
    
    for (int i = 0 ; i < rcnt; ++i) {
        rfile = rfiles.at(i).first;
        rattr = rfiles.at(i).second;
        found = -1;
        for (int j = 0; j < lfiles.count(); ++j) {
            lfile = lfiles.at(j).first;
            lattr = lfiles.at(j).second;

            // compare name
            if (lfile == rfile) {
                found = j;
                break;
            }
        }
        if (found != -1) {
            // compare time
            if (this->sameFileTime(rattr->mtime, lattr->mtime)) {
                // compare size
                if (rattr->filesize == lattr->filesize) {
                    rattr->flags |= FLAG_FILE_EQUAL;
                    lattr->flags |= FLAG_FILE_EQUAL;
                } else {
                    rattr->flags |= FLAG_FILE_DIFFERENT;
                    lattr->flags |= FLAG_FILE_DIFFERENT;
                }                
            } else {
                if (rattr->mtime > lattr->mtime) {
                    rattr->flags |= FLAG_REMOTE_NEWER;
                    lattr->flags |= FLAG_REMOTE_NEWER;
                } else {
                    rattr->flags |= FLAG_LOCAL_NEWER;
                    lattr->flags |= FLAG_LOCAL_NEWER;
                }
            }
            files.append(QPair<QString,LIBSSH2_SFTP_ATTRIBUTES*>(rfile,rattr));
            // TODO free the mem of lattr
            free(lattr); lattr = NULL;
            lfiles.remove(found);
        } else {
            rattr->flags |= FLAG_REMOTE_ONLY;
            files.append(QPair<QString,LIBSSH2_SFTP_ATTRIBUTES*>(rfile,rattr));
        }
    }

    lcnt = lfiles.count();
    if (lcnt > 0) {
        for (int j = 0; j < lcnt; ++j) {
            lfile = lfiles.at(j).first;
            lattr = lfiles.at(j).second;
            
            lattr->flags |= FLAG_LOCAL_ONLY;
            files.append(QPair<QString,LIBSSH2_SFTP_ATTRIBUTES*>(lfile,lattr));
        }
    }

    rfiles.clear();
    lfiles.clear();

    return files;
}

bool SyncWalker::dumpMergeResult(QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &files)
{
    QString file;
    LIBSSH2_SFTP_ATTRIBUTES *attr;
    QString fline;

    for(int i = 0 ; i < files.count(); ++i) {
        file = files.at(i).first;
        attr = files.at(i).second;

        fline = QString("%1").arg(i) + ": " + file + " [ ";
        if (attr->flags & FLAG_LOCAL_ONLY) {
            fline += QString("local only");
        } 
        if (attr->flags & FLAG_REMOTE_ONLY) {
            fline += QString("remote only");
        } 
        if (attr->flags & FLAG_LOCAL_NEWER) {
            fline += QString("local newer");            
        } 
        if (attr->flags & FLAG_REMOTE_NEWER) {
            fline += QString("remote newer");
        } 
        if (attr->flags & FLAG_FILE_EQUAL) {
            fline += QString("the same");
        } 
        if (attr->flags & FLAG_FILE_DIFFERENT) {
            fline += QString("???");
        }
        fline += " ] ";

        qDebug()<<fline;
    }
    return true;
}
QString SyncWalker::diffDesciption(unsigned long flags)
{
    QString fline;

    if (flags & FLAG_LOCAL_ONLY) {
        fline += QString("local only");
    } 
    if (flags & FLAG_REMOTE_ONLY) {
        fline += QString("remote only");
    } 
    if (flags & FLAG_LOCAL_NEWER) {
        fline += QString("local newer");            
    } 
    if (flags & FLAG_REMOTE_NEWER) {
        fline += QString("remote newer");
    } 
    if (flags & FLAG_FILE_EQUAL) {
        fline += QString("the same");
    } 
    if (flags & FLAG_FILE_DIFFERENT) {
        fline += QString("???");
    }

    return fline;
}
/*
 */
void SyncWalker::run()
{
    if (!this->checkLocalInfo()) {
        q_debug()<<"";
        return ;
    }
    if (!this->connectToRemoteHost() || !this->checkRemoteInfo()) {
        q_debug()<<"error";
        return ;
    }

    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > remoteFiles;
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > localFiles;
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > mergedFiles;

    remoteFiles = this->getRemoteFiles();
    localFiles = this->getLocalFiles();

    q_debug()<<"Remote File count: "<<remoteFiles.count();
    // q_debug()<<remoteFiles;
    q_debug()<<"File count: "<<localFiles.count();
    // q_debug()<<localFiles;
    q_debug()<<"merging...";
    mergedFiles = this->sortMerge(remoteFiles, localFiles);
    this->dumpMergeResult(mergedFiles);
    this->mMergedFiles = mergedFiles;
   
    q_debug()<<"thread end";
}

//////////////////////
/////////////////////
SynchronizeWindow::SynchronizeWindow(QWidget *parent, Qt::WindowFlags flags)
    :QWidget(parent, flags),
     ctxMenu(0)
{
    this->ui_win.setupUi(this);
    walker = new SyncWalker(this);
    QObject::connect(walker, SIGNAL(status_msg(QString)), this, SLOT(slot_status_msg(QString)));
    QObject::connect(walker, SIGNAL(finished()), this, SLOT(slot_finished()));
    QObject::connect(this->ui_win.toolButton_4, SIGNAL(clicked()), this, SLOT(start()));
    this->running = false;

    // model = new SyncDifferModel(this);
    // this->ui_win.treeView->setModel(model);
    // QObject::connect(walker, SIGNAL(found_row()), model, SLOT(maybe_has_data()));
    this->ui_win.treeView->setModel(0);    
    QObject::connect(&this->progress_timer, SIGNAL(timeout()), this, SLOT(progress_timer_timeout()));
    QObject::connect(this->ui_win.treeView, SIGNAL(customContextMenuRequested ( const QPoint & )),
                     this, SLOT(showCtxMenu(const QPoint &)));
}

SynchronizeWindow::~SynchronizeWindow()
{
    q_debug()<<"destructured";    
    if(this->walker->isRunning()) {
        q_debug()<<"walker thread is running";
    }
    delete this->walker;
}

void SynchronizeWindow::set_sync_param(QString local_dir, QString sess_name, QString remote_dir, bool recursive, int way)
{
    this->local_dir = local_dir;
    this->sess_name = sess_name;
    this->remote_dir = remote_dir;
    this->recursive = recursive;
    this->way = way;

    this->ui_win.lineEdit->setText(local_dir);
    this->ui_win.lineEdit_2->setText(sess_name + ":" + remote_dir);
}

void SynchronizeWindow::start()
{
    if(!this->running) {
        this->running = true;
        this->walker->start();
        this->progress_timer.start(100);
    }
    this->initCtxMenu();
}
void SynchronizeWindow::stop()
{
    
}
void SynchronizeWindow::slot_finished()
{
    this->running = false;
    model = new SyncDifferModel(this);
    model->setDiffFiles(this->walker->mMergedFiles);
    this->ui_win.treeView->setModel(model);
    // QObject::connect(walker, SIGNAL(found_row()), model, SLOT(maybe_has_data()));

    this->ui_win.treeView->expandAll();

    this->progress_timer.stop();
    this->ui_win.progressBar->setValue(100);
}

void SynchronizeWindow::slot_status_msg(QString msg)
{
    this->ui_win.label->setText(msg);
}

void SynchronizeWindow::closeEvent(QCloseEvent *evt)
{
    //q_debug()<<"";
    this->deleteLater();
    QWidget::closeEvent(evt);
}

QString SynchronizeWindow::diffDesciption(unsigned long flags)
{
    return this->walker->diffDesciption(flags);
}

void SynchronizeWindow::progress_timer_timeout()
{
    int val = this->ui_win.progressBar->value();
    if (val == 100) {
        val = 0;
    }
    val = val == 100 ? 0 : val + 5;
    this->ui_win.progressBar->setValue(val);
}

void SynchronizeWindow::initCtxMenu()
{
    if (this->ctxMenu == NULL) {
        this->ctxMenu = new QMenu(this);
        QAction *action;
        action = new QAction(tr("File info..."), this->ctxMenu);
        this->ctxMenu->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(showDiffFileInfo()));
    }
}

void SynchronizeWindow::showCtxMenu(const QPoint & pos)
{
    if (this->ctxMenu == NULL) {
        return ;
    }
    //if (this->model == NULL) {
    //  return ;
    //}
    QPoint adjPos = this->ui_win.treeView->mapToGlobal(pos);
    adjPos.setX(adjPos.x() + 5);
    this->ctxMenu->popup(adjPos);
}

void SynchronizeWindow::showDiffFileInfo()
{
    QString info;
    QItemSelectionModel *ism = NULL;
    QModelIndexList mil;
    QModelIndex idx ;
    QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> file;

    ism = this->ui_win.treeView->selectionModel();
    if (ism != NULL) {
        mil = ism->selectedIndexes();
        idx = mil.at(0);
        file = this->model->getFile(idx);
        Q_ASSERT(file.isValid());
        info = QString(tr("%1\nType: %2\nSize: %3\nLast modified: %4\nSync Status: %5"))
            .arg(file.first, 
                 SSHFileInfo(file.second).isDir() ? tr("Direcotry") : tr("File"),
                 QString("%1").arg(file.second->filesize),
                 QDateTime::fromTime_t(file.second->mtime).toString(),
                 this->walker->diffDesciption(file.second->flags)
                 );
    }

    QPoint pos = QCursor::pos();
    //QWhatsThis::showText(pos, tr("sdfsdfsdfdsf\nsdfijsdfiowej"));
    QWhatsThis::showText(pos, info);
}



