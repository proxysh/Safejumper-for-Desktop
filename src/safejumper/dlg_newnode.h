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

#ifndef DLG_NEWNODE_H
#define DLG_NEWNODE_H

#include <QDialog>

namespace Ui
{
class Dlg_newnode;
}

class Dlg_newnode : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_newnode(const QString & msg, QWidget *parent = 0);
    ~Dlg_newnode();

    bool IsCyclePort()
    {
        return _port;   // true - if cycle ports; false - cycle nodes
    }
private slots:
    void _ac_Cancel();
    void _ac_Confirm();
private:
    Ui::Dlg_newnode *ui;
    bool _port;
};

#endif // DLG_NEWNODE_H
