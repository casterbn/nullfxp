// curlftp_callback.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-19 20:55:34 +0800
// Version: $Id$
// 

#include <QtCore>

#include "curl/curl.h"

#include "curlftp.h"
#include "curlftp_callback.h"

// 给多少数据就要读取多少，读取不完不行。
size_t callback_read_dir(void *ptr, size_t size, size_t nmemb, void *userp)
{
    qDebug()<<__FILE__<<__LINE__<<__FUNCTION__<<size<<nmemb<<userp;
    
    size_t tlen = size * nmemb, rlen = 0;
    QBuffer strbuf;
    QByteArray line;
    int n = 0;

    strbuf.setData((const char*)ptr, tlen);
    strbuf.open(QBuffer::ReadOnly);
    Q_ASSERT(strbuf.canReadLine()); //  ???
    rlen = 0;
    while (!strbuf.atEnd()) {
        line = strbuf.readLine();
        rlen += line.length();
        qDebug()<<"Line: "<<n++<<line;
        // break;
    }
    strbuf.close();

    return rlen;
}

size_t callback_read_file(void *ptr, size_t size, size_t nmemb, void *userp)
{
    // qDebug()<<__FILE__<<__LINE__<<__FUNCTION__<<size<<nmemb<<userp;

    size_t tlen = size * nmemb, rlen = 0;
    QBuffer strbuf;
    QByteArray line;
    int n = 0, wlen = 0;

    CurlFtp *ftp = static_cast<CurlFtp*>(userp);
    QLocalSocket *router = ftp->getDataSock2();

    strbuf.setData((const char*)ptr, tlen);
    strbuf.open(QBuffer::ReadOnly);
    // Q_ASSERT(strbuf.canReadLine()); //  ???
    rlen = 0;
    while (!strbuf.atEnd()) {
        if (strbuf.canReadLine()) {
            line = strbuf.readLine();
        } else {
            line = strbuf.readAll();
        }
        rlen += line.length();
        wlen = router->write(line);
        // qDebug()<<"Line: "<<n++<<line.length()<<wlen;
        // fprintf(stdout, "%s", ".");
        // fflush(stdout);
        Q_ASSERT(line.length() == wlen);
        // break;
    }
    strbuf.close();
    router->flush();

    // qDebug()<<"can rw:"<<router->isReadable()<<router->isWritable()<<router->isOpen();
    // fprintf(stdout, "route read file:. %p %d %s", router, router->bytesAvailable(), "\n");
    fflush(stdout);

    return rlen;
    return 0;
}

int gn = 0;
size_t callback_write_file(void *ptr, size_t size, size_t nmemb, void *userp)
{
    // qDebug()<<__FILE__<<__LINE__<<__FUNCTION__<<size<<nmemb<<userp;

    int tlen = size * nmemb;
    char *s = (char*)ptr;
    int rlen = 0;

    CurlFtp *ftp = static_cast<CurlFtp*>(userp);
    QLocalSocket *router = ftp->getDataSock2();
    QLocalSocket *suppler = ftp->getDataSock();

    // qDebug()<<"here";
    Q_ASSERT(router != NULL);
    // qDebug()<<"here"<<router;

 retry_read:
    if (!ftp->isPutDataFinished() || router->bytesAvailable() > 0) {
        if (router->bytesAvailable() > 0) {
        } else {
            router->waitForReadyRead(1000);
            router = ftp->getDataSock2();
            // qDebug()<<"wait for ready read..."<<router->errorString()
            //         <<router->isOpen();
            if (router->bytesAvailable() == 0) {
                suppler = ftp->getDataSock();
                if (suppler == NULL) {
                    // suppler is finished.
                    // ftp->closeDataChannel2();
                    return 0;
                }
                // continue;
                goto retry_read;
            }
        }
        Q_ASSERT(router->bytesAvailable() > 0);
        rlen = router->read(s, tlen);
            
        if (rlen < 0) {
            Q_ASSERT(rlen >= 0);
            return 0;
        } else if (rlen == 0) {
            qDebug()<<"no data left";
            return 0;
        } else {
            return rlen;
        }
    } else {
        return 0;
    }
    return 0;

    // for (;;) {
    //     router = ftp->getDataSock2();
    //     if (router->bytesAvailable() > 0) {
    //     } else {
    //         router->waitForReadyRead(1000);
    //         router = ftp->getDataSock2();
    //         // qDebug()<<"wait for ready read..."<<router->errorString()
    //         //         <<router->isOpen();
    //         if (router->bytesAvailable() == 0) {
    //             suppler = ftp->getDataSock();
    //             if (suppler == NULL) {
    //                 // suppler is finished.
    //                 // ftp->closeDataChannel2();
    //                 return 0;
    //             }
    //             continue;
    //         }
    //     }
    //     Q_ASSERT(router->bytesAvailable() > 0);
    //     rlen = router->read(s, tlen);
            
    //     if (rlen < 0) {
    //         Q_ASSERT(rlen >= 0);
    //         return 0;
    //     } else if (rlen == 0) {
    //         qDebug()<<"no data left";
    //         return 0;
    //     } else {
    //         return rlen;
    //     }
    // }

    // rlen = router->read(s, tlen);
    // if (rlen < 0) {
    //     Q_ASSERT(rlen >= 0);
    //     return 0;
    // } else if (rlen == 0) {
    //     return 0;
    // } else {
    //     return rlen;
    // }

    if (gn == 0) {
        gn ++;
    } else {
        return 0;
    }
    for (int i = 0 ; i < tlen ; i ++) {
        s[i] = 'v';
    }

    return tlen;
}

size_t callback_write_file_via_proxy(void *ptr, size_t size, size_t nmemb, void *userp)
{
    // qDebug()<<__FILE__<<__LINE__<<__FUNCTION__<<size<<nmemb<<userp;

    int tlen = size * nmemb;
    char *s = (char*)ptr;
    int rlen = 0;

    CurlFtp *ftp = static_cast<CurlFtp*>(userp);
    QLocalSocket *router = ftp->getDataSock2();
    QLocalSocket *suppler = ftp->getDataSock();
    
    // qDebug()<<"here";
    Q_ASSERT(router != NULL);
    // qDebug()<<"here"<<router;

    return 0;

    if (gn == 0) {
        gn ++;
    } else {
        return 0;
    }
    for (int i = 0 ; i < tlen ; i ++) {
        s[i] = 'v';
    }

    return tlen;
}

int callback_debug(CURL *curl, curl_infotype it, char *text, size_t size, void *userp)
{
    char *str = (char*)calloc(1, size + 1);
    memset(str, 0, size + 1);
    // strncpy(str, text, size);
    memcpy(str, text, size);
    str[size] = '\0';
    
    QString tmp(str);
    tmp = tmp.trimmed() + '\n';      // str has \r\n, replace with \n
    strcpy(str, tmp.toAscii().data());
    qDebug()<<"spy debug:"<<it<<str;
    
    // if (QString(str).startsWith("drwxr-xr-x")) {
    //     for (int i = 0 ; i < strlen(str) ; ++ i) {
    //         printf("%c %d \n", str[i], str[i]);
    //     }
    //     exit(0);
    // }

    if (it == CURLINFO_HEADER_IN) {
        CurlFtp *ftp = static_cast<CurlFtp*>(userp);
        ftp->rawRespBuff.write(str, strlen(str));
    }

    free(str);
    return 0;
}

CURLcode callback_sslctxfun(CURL *curl, void *sslctx, void *parm)
{
    CURLcode res;

    qDebug()<<__FILE__<<__LINE__<<__FUNCTION__<<curl<<sslctx<<parm;    

    return res;
}

