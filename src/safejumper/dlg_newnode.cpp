/***************************************************************************
 *   Copyright (C) 2017 by Jeremy Whiting <jeremypwhiting@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2 of the License.                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "dlg_newnode.h"
#include "ui_dlg_newnode.h"
#include "wndmanager.h"
#include "fonthelper.h"

Dlg_newnode::Dlg_newnode(const QString & msg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_newnode)
{
    ui->setupUi(this);
#ifndef Q_OS_DARWIN
    FontHelper::SetFont(this);
#endif
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    WndManager::DoShape(this);

    this->setAttribute(Qt::WA_MacNoShadow, false);
    ui->L_Text->setText(msg);
}

Dlg_newnode::~Dlg_newnode()
{
    delete ui;
}

void Dlg_newnode::_ac_Cancel()
{
    _port = false;
    this->accept();
}

void Dlg_newnode::_ac_Confirm()
{
    _port = true;
    this->accept();
}
