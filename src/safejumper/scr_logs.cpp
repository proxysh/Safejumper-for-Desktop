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

#include "scr_logs.h"

#include <QClipboard>

#include "ui_scr_logs.h"
#include "connectiondialog.h"
#include "common.h"
#include "wndmanager.h"
#include "fonthelper.h"
#include "log.h"
#include "loginwindow.h"

Scr_Logs::Scr_Logs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Scr_Logs)
{
#ifdef Q_OS_WIN
    setWindowFlags(Qt::Dialog);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

    ui->setupUi(this);
    this->setFixedSize(this->size());
#ifndef Q_OS_DARWIN
    FontHelper::SetFont(this);
#endif
    connect(Log::instance(), &Log::logMessage,
            this, &Scr_Logs::addLogMessage);
}

void Scr_Logs::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

Scr_Logs::~Scr_Logs()
{
    delete ui;
}

std::auto_ptr<Scr_Logs> Scr_Logs::_inst;
Scr_Logs * Scr_Logs::Instance()
{
    if (!_inst.get())
        _inst.reset(new Scr_Logs());
    return _inst.get();
}

bool Scr_Logs::IsExists()
{
    return (_inst.get() != NULL);
}

void Scr_Logs::Cleanup()
{
    if (_inst.get() != NULL) delete _inst.release();
}

void Scr_Logs::ShowSupportUrl()
{
    OpenUrl_Support();
}

void Scr_Logs::ToScr_Connect()
{
    WndManager::Instance()->ToPrimary();
}

void Scr_Logs::addLogMessage(const QString & s)
{
    QTextCursor c = ui->e_Logs->textCursor();
    c.movePosition(QTextCursor::End);
    c.insertText(s);
    c.movePosition(QTextCursor::End);
    int n = ui->e_Logs->toPlainText().length();
    c.setPosition(n);
    ui->e_Logs->setTextCursor(c);
}

void Scr_Logs::Clicked_Copytoclipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->e_Logs->toPlainText());
}

void Scr_Logs::keyPressEvent(QKeyEvent * e)
{
    if(e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}
