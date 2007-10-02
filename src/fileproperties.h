/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
 *
 *   http://www.qtchina.net                                                *
 *   http://nullget.sourceforge.net                                        *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef FILEPROPERTIES_H
#define FILEPROPERTIES_H

#include <QtCore>
#include <QtGui>
#include <QDialog>

#include "ui_fileproperties.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class FileProperties : public QDialog
{
Q_OBJECT
public:
    FileProperties(QWidget *parent = 0);

    ~FileProperties();
    
    void set_file_info_model_list(QModelIndexList &mil);
    
    private:
        Ui::FileProperties ui_file_prop_dialog;
        
};

#endif