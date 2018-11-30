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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"

#include <QNetworkReply>
#include <QQuickView>

class QNetworkAccessManager;
class QProgressDialog;

class MainWindow : public QQuickView
{
    Q_OBJECT
public:

    ~MainWindow();
    static bool exists();
    static MainWindow *instance();
    static void cleanup();

    void showFeedback();
    void showConnection();

    Q_INVOKABLE const QString logsContent() const;

    Q_INVOKABLE void copyLogsToClipboard() const;
    Q_INVOKABLE void launchCustomerService() const;

public slots:
    void portDialogResult(bool cyclePort);

    void showAndFocus();
    void showLogs();
    void showMap();
    void showSettings();

    void languageChanged();

    void showConfirmation(const QString &text);

    void closeWindow();

    void shutDown();

    void launchUrl(const QString &url);

signals:
    void confirmExit();
    void logout();
    void mapScreen();
    void settingsScreen();
    void logsScreen();

protected:
//    virtual bool event(QEvent *ev) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private slots:
    // Feedback page slots
    void on_sendFeedbackButton_clicked();
    void on_cancelFeedbackButton_clicked();

    void postError(QNetworkReply::NetworkError error);
    void sendFeedbackFinished();

    void logoutTriggered();

    void exitTriggered();

    void messageReceived(const QString &message);

private:
    static QPointer<MainWindow> mInstance;
    explicit MainWindow();

    // Feedback page members
    QNetworkAccessManager *mNam;
    QProgressDialog *mProgressDialog;
};


#endif // CONNECTIONDIALOG_H
