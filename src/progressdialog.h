/* progressdialog.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2008-05-06 22:13:23 +0800
 * Version: $Id$
 */

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QtCore>
#include <QtGui>
#include <QDialog>

#include "taskpackage.h"
// #include "transportor.h"

class Transportor;
class TaskQueueModel;

namespace Ui {
    class ProgressDialog;
};

class ProgressDialog : public QWidget
{
    Q_OBJECT;
public:    
    ProgressDialog(QWidget *parent = 0);
    virtual ~ProgressDialog();

    // local_pkg is src pkg
    // remote_pkg is dest pkg ???
    void set_transfer_info(TaskPackage local_pkg, TaskPackage remote_pkg);
    
public slots:
    void slot_set_transfer_percent(int percent, quint64 total_transfered, int transfer_delta);
    void slot_transfer_thread_finished();
    void slot_new_file_transfer_started(QString new_file_name);
        
    void exec();
    void show();
    void slot_cancel_button_clicked();
    void slot_transfer_got_file_size(quint64 size);
    void slot_transfer_log(QString log);
    void slot_dest_file_exists(QString src_path, QString src_file_size, 
                               QString src_file_date, QString dest_path,
                               QString dest_file_size, QString dest_file_date);
    void slot_ask_accepted(int which);

private slots:
    void slot_speed_timer_timeout();
    void slot_refresh_ui_timer_timeout();

signals:
    void transfer_finished(int status, QString errorString);
        
private:
    TaskPackage local_pkg;
    TaskPackage remote_pkg;

    Transportor *transportor;
    bool   first_show;
    TaskQueueModel *taskQueueModel;

    quint64 total_transfered;
    quint64 total_files_size;
    quint64 total_abtained_size;
    int     total_files_count;
    quint64 abtained_files_count;
    quint64 curr_file_size;
    quint64 curr_abtained_size;
    int  transfer_speed;
    QDateTime start_time;
    QDateTime end_time;
    QTimer  time_cacl_timer;
    QTimer  refresh_ui_timer; // random trigger internal, 300-1200
        
private:    //UI element
    Ui::ProgressDialog *uiw; 
    int modelId;

private:
    void update_transfer_state();
    QString type(QString file_name);
    inline int rand_intval() { return qrand() % 900 + 300; }

protected:
    void closeEvent(QCloseEvent *event);
};

#endif
