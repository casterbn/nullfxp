// sshproxy.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-07-19 17:25:21 +0000
// Version: $Id$
// 

#ifndef _SSHPROXY_H_
#define _SSHPROXY_H_

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

namespace Ui {
    class SSHProxy;
};

class SSHProxy : public QDialog
{
    Q_OBJECT;
public:
    explicit SSHProxy(QWidget *parent = 0);
    virtual ~SSHProxy();

public slots:
    void slot_start();

private:
    Ui::SSHProxy *uiw;
    QTcpServer *mserv;
};

#endif /* _SSHPROXY_H_ */
