/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   gzl@localhost   *
 *                                                                         *
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
#include <cassert>

#include <QTextCodec>

#include "globaloption.h"

GlobalOption * global_option = GlobalOption::instance();

GlobalOption * GlobalOption::mInstance = 0 ;

GlobalOption * GlobalOption::instance()
{
    if(GlobalOption::mInstance == 0 )
    {
        GlobalOption::mInstance = new GlobalOption();
        GlobalOption::mInstance->remote_codec = QTextCodec::codecForName("UTF-8");
        //GlobalOption::mInstance->locale_codec = "UTF-8";
        GlobalOption::mInstance->locale_codec = QTextCodec::codecForLocale();
        GlobalOption::mInstance->keep_alive = true ;
        GlobalOption::mInstance->kepp_alive_internal = 180; //S
    }
    return GlobalOption::mInstance;
}
GlobalOption::GlobalOption()
{
}


GlobalOption::~GlobalOption()
{
}

void GlobalOption::set_remote_codec( const char * codec )
{
    this->remote_codec = QTextCodec::codecForName(codec);
    if( this->remote_codec == 0 )
    {
        this->remote_codec = QTextCodec::codecForName("UTF-8");
        assert( 1 == 2 );
    }
}

