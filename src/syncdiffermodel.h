/* syncdiffermodel.h --- 
 * 
 * Filename: syncdiffermodel.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 日  8月 10 21:13:22 2008 (CST)
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

#include <QtCore>
#include <QtGui>

#include "libssh2.h"
#include "libssh2_sftp.h"

class SynchronizeWindow;

class SyncDifferModel : public QAbstractItemModel
{
    Q_OBJECT;
public:
    SyncDifferModel(QObject *parent = 0);
    ~SyncDifferModel();

    //5个必须实现的虚函数
    QModelIndex index(int, int, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QVariant   data(const QModelIndex &index, int role = Qt::DisplayRole ) const;
    int rowCount(const QModelIndex &index) const;
    int columnCount(const QModelIndex &index) const;

    //
    bool setDiffFiles(QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > files);
    QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> getFile(const QModelIndex & index) const;

public slots:
    void maybe_has_data();

private:
    SynchronizeWindow *sync_win;
    QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > mMergedFiles;
    QVector<QString>  mTransferStatus;
};
