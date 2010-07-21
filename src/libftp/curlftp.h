// curlftp.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-18 16:25:05 +0800
// Version: $Id$
// 


#ifndef _CURLFTP_H_
#define _CURLFTP_H_

#include <QtCore>
#include <QtNetwork>

#include "curl/curl.h"

class CurlFtp : public QThread
{
    Q_OBJECT;
public:
    explicit CurlFtp(QObject *parent = 0);
    virtual ~CurlFtp();

    enum {TYPE_MIN = 0, TYPE_ASCII, TYPE_EBCID, TYPE_BIN,
          TYPE_IMAGE, TYPE_LOCAL_BYTE,
          TYPE_MAX};
    enum {FE_MIN = 0, FE_FTP, FE_FTPS, FE_FTPES, FE_FTP_OVER_SSH, FE_MAX};

    /*
      http://en.wikipedia.org/wiki/FTPS
      FTPS FTP over SSL,  adds SSL or TLS encryption to FTP as specified in RFC 4217
      FTP Over ssh, Secure FTP, the practice of tunneling FTP through an SSH connection.
     */

    int connect(const QString host, unsigned short port = 21);
    int login(const QString &user = QString(), const QString &password = QString());
    int get();
    int put();

    int connectDataChannel();
    int closeDataChannel();
    int list(QString path);
    int lista(QString path);
    int nlist(QString path);
    int mlst(QString path);
    int pwd(QString &path); // returned path
    int mkdir(QString path);
    int rmdir(QString path);
    int chdir(QString path);
    int put(const QString fileName);
    int putNoWaitResponse(const QString fileName);
    int get(const QString fileName);
    int getNoWaitResponse(const QString fileName);
    int remove(const QString path);
    int rename(const QString src, const QString dest);
    int passive();
    int rein(const QString &user = QString(), const QString &password = QString()); // 重新登陆
    int type(int type);
    int noop();
    int system(QString &type);
    int stat(QString path);
    int port(const QString hostip, const unsigned short port); // fxp
    int portNoWaitResponse(const QString hostip, const unsigned short port);
    int size(QString path, quint64 &siz);

    QLocalSocket *getDataSock();
    QLocalSocket *getDataSock2();

    int getInfoDemo();

protected:
    virtual void run();

signals:
    void runHere();

public slots:
    void nowarnRunTask();
    void nowarnRunDone(); 
    void asynRunRetrDone();

private:
    QString proxyName();

public:
    // QThread *retrTask;

private:
    CURL *curl;
    CURLSH *shareHandle;
    QString baseUrl; // ftp://host:port part
    QString listDirPath;
    QString upFilePath;
    QString dlFilePath;
    QLocalServer *curlWriteDataRouteServer;
    QLocalSocket *qdsock;
    QLocalSocket *qdsock2;
    static int seq;
};

#endif
