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

#define _WINSOCKAPI_
#include "mainwindow.h"

#include "authmanager.h"
#include "common.h"
#include "wndmanager.h"
#include "setting.h"
#include "vpnservicemanager.h"
#include "pathhelper.h"
#include "log.h"

#include <QHttpMultiPart>
#include <QMessageBox>
#include <QProgressDialog>
#include <QQmlContext>
#include <QQmlEngine>

const int kConnectionPage = 0;
const int kFeedbackPage = 1;

MainWindow::MainWindow() :
    QQuickView(),
    mNam(nullptr),
    mProgressDialog(nullptr)
{
    setSource(QUrl("qrc:/qml/MainWindow.qml"));

    rootContext()->setContextProperty("authmanager", AuthManager::instance());
    rootContext()->setContextProperty("serversModel", AuthManager::instance()->serversModel());
    rootContext()->setContextProperty("settings", Setting::instance());

    connect(Setting::instance(), &Setting::languageChanged,
            this, &MainWindow::languageChanged);

    // Call it here, since the language was loaded by the above instantiation
    // of the setting object
    languageChanged();

    setMaximumHeight(755);
    setMinimumHeight(755);
    setMaximumWidth(375);
    setMinimumWidth(375);

    setFlags(Qt::Dialog);
    setIcon(QIcon(":/images/logo.png"));

    // Setting::Instance()->LoadServer();
    Setting::instance()->loadProtocol();

    if (Setting::instance()->autoconnect())
        AuthManager::instance()->login(Setting::instance()->login(), Setting::instance()->password());
}

bool MainWindow::exists()
{
    return (!mInstance.isNull());
}

void MainWindow::cleanup()
{
    if (!mInstance.isNull())
        mInstance->deleteLater();
}

MainWindow::~MainWindow()
{
    {
        if (this->isVisible()) {
//            WndManager::Instance()->HideThis(this);
//            WndManager::Instance()->SavePos();
        }
    }
//    delete ui;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    event->ignore();
//    WndManager::Instance()->HideThis(this);
}

void MainWindow::showFeedback()
{
//    ui->titleLineEdit->clear();
//    ui->feedbackTextEdit->clear();

//    ui->stackedWidget->setCurrentIndex(kFeedbackPage);
}

void MainWindow::showConnection()
{
//    ui->stackedWidget->setCurrentIndex(kConnectionPage);
}

QPointer<MainWindow> MainWindow::mInstance;
MainWindow * MainWindow::instance()
{
    if (mInstance.isNull()) {
        mInstance = new MainWindow();
    }
    return mInstance;
}

void MainWindow::portDialogResult(int action)
{
    if (QDialog::Accepted == action) {
        // Switch to next port/node and tell service to connect
        VPNServiceManager::instance()->startPortLoop(WndManager::Instance()->IsCyclePort());
    }
}

void MainWindow::showAndFocus()
{
    show();
    raise();
//    activateWindow();
}

void MainWindow::showLogs()
{
    // Manipulate qml object to show logs in popup
}

void MainWindow::showMap()
{

}

void MainWindow::showSettings()
{
    // Manipulate qml object to show settings page
}

void MainWindow::languageChanged()
{
    rootContext()->engine()->retranslate();
}

void MainWindow::on_cancelFeedbackButton_clicked()
{
    showConnection();
}

void MainWindow::on_sendFeedbackButton_clicked()
{
    // Gather the information we need to send
    // Username, log file(s), feedback text
//    QString title = ui->titleLineEdit->text();

//    if (title.isEmpty()) {
//        QMessageBox::warning(this, "Blank title", "Title field is required", QMessageBox::Ok);
//        ui->titleLineEdit->setFocus();
//        return;
//    }

    QString email = AuthManager::instance()->email();
    QString loginName = AuthManager::instance()->VPNName();

    QString feedbackText; // = ui->feedbackTextEdit->toPlainText();
    QFile debugLog(PathHelper::Instance()->safejumperLogFilename());
    debugLog.open(QIODevice::ReadOnly|QIODevice::Text);
    // Only send last 4k of log
    QString logText = QString(debugLog.readAll()).right(4096);

    // Construct post
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart loginPart;
    loginPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"login\""));
    loginPart.setBody(loginName.toLatin1());

    QHttpPart emailPart;
    emailPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"email\""));
    emailPart.setBody(email.toLatin1());

//    QHttpPart titlePart;
//    titlePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"title\""));
//    titlePart.setBody(title.toLatin1());

    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"text\""));
    textPart.setBody(feedbackText.toLatin1());

    QHttpPart logPart;
    logPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"logtext\""));
    logPart.setBody(logText.toLatin1());

    multiPart->append(loginPart);
    multiPart->append(emailPart);
//    multiPart->append(titlePart);
    multiPart->append(textPart);
    multiPart->append(logPart);

    QNetworkRequest request(QUrl("https://proxy.sh/api-feedback.php"));
    request.setRawHeader("cache-control:", "no-cache");

    if (mNam)
        mNam->deleteLater();
    mNam = new QNetworkAccessManager(this);
//    request.setRawHeader(QByteArray("UDID"), Log::udid().toLatin1());
    QNetworkReply *reply = mNam->post(request, multiPart);
    multiPart->setParent(reply);

    bool result = connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                          this, &MainWindow::postError);
    qDebug() << "result of connecting to error signal is " << result;
    result = connect(reply, &QNetworkReply::finished,
                     this, &MainWindow::sendFeedbackFinished);
    qDebug() << "result of connecting to finished signal is " << result;
    qDebug() << "Sent feedback with title "
             << " login " << loginName
             << " email " << email
             << " awaiting response";

    if (mProgressDialog) {
        mProgressDialog->close();
        mProgressDialog->deleteLater();
        mProgressDialog = nullptr;
    }

    mProgressDialog = new QProgressDialog();
    mProgressDialog->setLabelText("Sending Bug Report\nPlease wait...");
    mProgressDialog->setRange(0, 0);
    mProgressDialog->setMinimumDuration(0);
    mProgressDialog->setValue(0);
}

void MainWindow::postError(QNetworkReply::NetworkError error)
{
    if (mProgressDialog) {
        mProgressDialog->close();
        mProgressDialog->deleteLater();
        mProgressDialog = nullptr;
    }

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    qDebug() << "Got error from post request " << error
             << " request url was " << reply->url().toString();
    QByteArray response = reply->readAll();
//    QMessageBox::information(this, "Post error", response);
}

void MainWindow::sendFeedbackFinished()
{
    if (mProgressDialog) {
        mProgressDialog->close();
        mProgressDialog->deleteLater();
        mProgressDialog = nullptr;
    }

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError) {
//        QMessageBox::information(this, "send feedback error", reply->errorString());
        return;
    }

    QByteArray response = reply->readAll();

    qDebug() << "issue response: " << response;

//    QMessageBox::information(this, "Issue created", QString("Issue created."));

    showConnection();
}

