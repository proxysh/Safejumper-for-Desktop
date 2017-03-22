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

#include "confirmationdialog.h"

#include "ui_confirmationdialog.h"
#include "wndmanager.h"
#include "fonthelper.h"

ConfirmationDialog::ConfirmationDialog(const QString & message, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfirmationDialog)
{
    ui->setupUi(this);
#ifndef Q_OS_DARWIN
    FontHelper::SetFont(this);
#endif
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    WndManager::DoShape(this);

    this->setAttribute(Qt::WA_MacNoShadow, false);
    ui->messageLabel->setText(message);
}

ConfirmationDialog::~ConfirmationDialog()
{
    delete ui;
}

void ConfirmationDialog::on_cancelButton_clicked()
{
    QDialog::reject();
}

void ConfirmationDialog::on_confirmButton_clicked()
{
    QDialog::accept();
}
