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

#include "errordialog.h"

#include "ui_errordialog.h"

#include "common.h"
#include "wndmanager.h"
#include "fonthelper.h"

ErrorDialog::ErrorDialog(const QString & msg, const QString & caption, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
#ifndef Q_OS_DARWIN
    FontHelper::SetFont(this);
#endif
    WndManager::DoShape(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    this->setAttribute(Qt::WA_MacNoShadow, false);
    ui->captionLabel->setText(caption);
    ui->textLabel->setText(msg);
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

void ErrorDialog::resizeEvent(QResizeEvent * )
{
    // on resize: it harms shape
    //WndManager::DoShape(this);
}

void ErrorDialog::on_closeButton_clicked()
{
    accept();
}

void ErrorDialog::on_supportButton_clicked()
{
    OpenUrl_Support();
    accept();
}
