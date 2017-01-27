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

#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include "common.h"

#include <QDialog>
#include <QProcess>
#include <memory>
#include <QTcpSocket>
#include <QTimer>

namespace Ui
{
    class TestDialog;
}

class TestDialog : public QDialog
{
    Q_OBJECT

public:

    ~TestDialog();
    static bool exists();
    static TestDialog * instance();
    static void cleanup();

    void SetServer(int srv);	// -1 or id inside all servers
    void UpdNewIp(const QString & s);
    void UpdEnc();

    void SetOldIp(const QString & ip);
    void SetAccName(const QString & s);
    void SetEmail(const QString & s);
    void SetAmount(const QString & s);
    void SetUntil(const QString & date);

    void SetProtocol(int ix);   // -1 for none

    void SetVpnName(const QString & vpnname);
    void StatusConnecting();
    void StatusConnecting(const QString & word);
    void StatusConnected();
    void StatusDisconnected();

public slots:
    void PortDlgAction(int action);

protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_startButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::TestDialog *ui;
    static std::auto_ptr<TestDialog> mInstance;
    explicit TestDialog(QWidget *parent = 0);
    void Init();

    void SetFlag(int srv);
    void SetNoSrv();
    void UpdProtocol();
    void SetEnabledButtons(bool enabled);

    bool _moving;
    void DwnlStrs();
    QPoint _WndStart;
    QPoint _CursorStart;

    static void InitStateWords();
    typedef QHash<QString, const char *> HmWords;
    static HmWords _StateWord_Img;

    int mCurrentEncryptionType;
    int mCurrentServerId;
    int mCurrentProtocol;
    std::vector<size_t> mEncryptionTypes; // List of all encryption types to iterate over
    std::vector<size_t> mServerIds; // List of current encryption type server ids
    std::vector<int> mProtocols; // List of current encryption type protocols
};


#endif // SCR_CONNECT_H
