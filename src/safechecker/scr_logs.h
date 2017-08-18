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

#ifndef SCR_LOGS_H
#define SCR_LOGS_H

#include <QDialog>
#include <QCloseEvent>
#include <memory>

namespace Ui
{
class Scr_Logs;
}

class Scr_Logs : public QDialog
{
    Q_OBJECT

public:
    ~Scr_Logs();
    static Scr_Logs * Instance();
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }
    static void Cleanup()
    {
        if (_inst.get() != NULL) delete _inst.release();
    }

    void Log(const QString & s);
public slots:
    void Clicked_Copytoclipboard();
private slots:
    void ShowSupportUrl();
    void ToScr_Connect();

protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);
private:
    Ui::Scr_Logs *ui;
    explicit Scr_Logs(QWidget *parent = 0);
    static std::auto_ptr<Scr_Logs> _inst;
    bool _moving;
    QPoint _WndStart;
    QPoint _CursorStart;
};

#endif // SCR_LOGS_H
