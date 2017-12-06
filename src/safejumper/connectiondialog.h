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

#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include "common.h"

#include <memory>
#include <QDialog>
#include <QNetworkReply>
#include <QProcess>
#include <QTcpSocket>
#include <QTimer>

class QNetworkAccessManager;
class QProgressDialog;

namespace Ui
{
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT
public:

    ~ConnectionDialog();
    static bool exists();
    static ConnectionDialog *instance();
    static void cleanup();

    void statusConnecting();
    void statusConnecting(const QString & word);
    void statusConnected();
    void statusDisconnected();

    void showFeedback();
    void showConnection();

public slots:
    void portDialogResult(int action);

protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);

private slots:
    void on_connectButton_clicked();
    void on_cancelButton_clicked();
    void on_jumpButton_clicked();
    void on_settingsButton_clicked();

    void showMapWindow();
    void showPackageUrl();

    void updateEncryption();
    void updateProtocol();
    void updateServer();

    void setOldIP(const QString & ip);
    void setEmail(const QString & s);
    void setAmount(const QString & s);
    void setUntil(const QString & date);
    void setNewIP(const QString & s);

    // Feedback page slots
    void on_sendFeedbackButton_clicked();
    void on_cancelFeedbackButton_clicked();

    void postError(QNetworkReply::NetworkError error);
    void sendFeedbackFinished();

private:
    Ui::ConnectionDialog *ui;
    static std::auto_ptr<ConnectionDialog> mInstance;
    explicit ConnectionDialog(QWidget *parent = 0);

    void setFlag(int srv);
    void setServer(int srv);	// -1 or id inside all servers
    void setNoServer();
    void setProtocol(int ix);   // -1 for none
    void enableButtons(bool enabled);

    static void initializeStateWords();
    typedef QHash<QString, const char *> HmWords;
    static HmWords mStateWordImages;

    // Feedback page members
    QNetworkAccessManager *mNam;
    QProgressDialog *mProgressDialog;
};


#endif // CONNECTIONDIALOG_H
