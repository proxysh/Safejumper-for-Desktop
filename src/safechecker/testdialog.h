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

public slots:
    void PortDlgAction(int action);
    void setAccountName(const QString & s);
    void setStatusConnecting();
    void setStatusConnecting(const QString & word);
    void setStatusConnected();
    void setStatusDisconnected();
    void setError(const QString &message);

protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);

private slots:
    void on_startButton_clicked();
    void on_pauseButton_clicked();
    void on_cancelButton_clicked();
    void on_saveCSVButton_clicked();

    void setOldIP(const QString & ip);
    void setEmail(const QString & s);
    void setAmount(const QString & s);
    void setUntil(const QString & date);
    void setNewIP(const QString & s);

    void updateServer(); // Slot for Setting::serverChanged
    void updateEncryption(); // Slot for Setting::encryptionChanged
    void updateProtocol(); // Slot for Setting::protocolChanged
private:
    Ui::TestDialog *ui;
    static std::auto_ptr<TestDialog> mInstance;
    explicit TestDialog(QWidget *parent = 0);

    void setFlag(int srv);
    void setServer(int srv);	// -1 or id inside all servers
    void setNoServer();
    void setProtocol(int ix);   // -1 for none

    // Record current encryption/server/protocol result and go to the next one
    void iterate(bool skipPorts = false);
    void nextServer(); // Iterate to the next server that doesn't have "Hub" in it's name
    // Add a row to the table widget
    int addRow();
    // Add a "connected" line to the table widget
    void addConnected();
    void addError(QString message);

    bool saveCSV(QString filename);

    static void initializeStateWords();
    static QHash<QString, const char*> mStateWordImages;

    int mCurrentEncryptionType;
    int mCurrentServerId;
    int mCurrentProtocol;
    std::vector<size_t> mEncryptionTypes; // List of all encryption types to iterate over
    QList<int> mServerIds; // List of current encryption type server ids
    std::vector<QString> mProtocols; // List of current encryption type protocols
    QString mLogFolder; // Folder to keep failure logs
};


#endif // TESTDIALOG_H
