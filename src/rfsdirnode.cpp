// rfsdirnode.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-08-09 11:50:49 +0000
// Last-Updated: 2009-07-13 22:43:00 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "utils.h"

#include "rfsdirnode.h"

////////////////////////directory_tree_item
directory_tree_item::~directory_tree_item()
{
    //qDebug()<<"tree delete now";
    int line = this->child_items.size();
    for (int i = line -1 ; i >=0 ; i --) {
        delete this->child_items[i];
    }
}

bool directory_tree_item::isDir()
{
    return S_ISDIR(this->attrib.permissions) || S_ISLNK(this->attrib.permissions);
}
int directory_tree_item::childCount()
{
    return this->child_items.size();
    return 0;
}
directory_tree_item *directory_tree_item::parent()
{
    return this->parent_item;
}
bool directory_tree_item::hasChild(QString name)
{
    for (int i = 0 ; i < this->child_items.size(); i++) {
        if (child_items[i]->file_name == name) {
            return true;
        }
    }
    
    return false;
}

bool directory_tree_item::setDeleteFlag(QString name, bool del)
{
    for (int i = 0 ; i < this->child_items.size(); i++) {
        if (child_items[i]->file_name == name) {
            this->child_items[i]->delete_flag = del;
            return true;
        }
    } 
    
    return false;
}
directory_tree_item *directory_tree_item::childAt(int index)
{
    return this->child_items[index];
}
QString directory_tree_item::filePath()
{
    return this->strip_path;
}
QString directory_tree_item::fileName()
{
    return this->file_name;
}
QString directory_tree_item::fileMode()
{
    return QString();
}
QString directory_tree_item::fileMDate()
{
    return QString();
}
QString directory_tree_item::fileADate()
{
    return QString();
}

