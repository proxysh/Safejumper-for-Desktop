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

#include "testdialog.h"

#include "ui_testdialog.h"
#include "loginwindow.h"
#include "authmanager.h"
#include "pathhelper.h"
#include "common.h"
#include "wndmanager.h"
#include "setting.h"
#include "openvpnmanager.h"
#include "log.h"
#include "flag.h"
#include "fonthelper.h"

#include <QDir>
#include <QFileDialog>

#include <algorithm>
#include <random>

const QString kLastFilenameKey = "lastFilename";

static const QString kPauseText = "PAUSE TEST";
static const QString kResumeText = "RESUME TEST";
static const QString kStartText = "FULL TEST";
static const QString kResetText = "RESET TEST";

static const int kServerColumn = 0;
static const int kResultColumn = 1;
static const int kEncryptionColumn = 2;
static const int kPortColumn = 3;

QHash<QString, const char*> TestDialog::mStateWordImages;
std::auto_ptr<TestDialog> TestDialog::mInstance;

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog),
    mQuickTest(false)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    ui->countryLabel->setText("");
    ui->L_Percent->setText("0%");
    ui->tableWidget->horizontalHeader()->resizeSection(kPortColumn, 4); // Set port column to small size and let it stretch

    ui->pauseButton->hide();

    setNoServer();

    mLogFolder = QDir::homePath() + "/.safecheckerlogs";
    qDebug() << "log folder is " << mLogFolder;

    setWindowFlags(Qt::Dialog);
#ifndef Q_OS_MAC
    FontHelper::SetFont(this);
#ifdef Q_OS_WIN
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->L_Until->setFont(FontHelper::pt(7));
    ui->L_Email->setFont(FontHelper::pt(10));
#else		// linux
    ui->L_Until->setFont(FontHelper::pt(7));
    ui->L_Package->setFont(FontHelper::pt(10));
#endif
//    QPoint p1 = ui->L_LOAD->pos();
//    p1.setX(p1.x() + 10);
//    ui->L_LOAD->move(p1);
#endif

    setStatusDisconnected();

    ui->L_Until->setText("active until\n-");
    ui->L_Amount->setText("-");

    // Setting::Instance()->LoadServer();
    Setting::instance()->loadProtocol();

    updateEncryption();
    updateProtocol();

    connect(AuthManager::instance(), &AuthManager::emailLoaded,
            this, &TestDialog::setEmail);
    connect(AuthManager::instance(), &AuthManager::untilLoaded,
            this, &TestDialog::setUntil);
    connect(AuthManager::instance(), &AuthManager::amountLoaded,
            this, &TestDialog::setAmount);

    connect(Setting::instance(), &Setting::serverChanged,
            this, &TestDialog::updateServer);
    connect(Setting::instance(), &Setting::encryptionChanged,
            this, &TestDialog::updateEncryption);
    connect(Setting::instance(), &Setting::protocolChanged,
            this, &TestDialog::updateProtocol);
}

void TestDialog::startTest()
{
    ui->pauseButton->setText(kPauseText);
    // Make and clean out log folder
    QDir dir(mLogFolder);
    if (!dir.exists())
        dir.mkdir(mLogFolder);
    else if (!dir.entryList(QDir::NoDotAndDotDot).isEmpty()) {
        QStringList filenames = dir.entryList(QDir::NoDotAndDotDot);
        foreach(QString filename, filenames)
            dir.remove(filename);
    }

    ui->startButton->setText(kResetText);
    ui->pauseButton->show();
    ui->quickTestButton->hide();
    ui->saveCSVButton->setEnabled(false);

    // Clear out previous results if any
    ui->tableWidget->setRowCount(0);
    // Get all encryption types
    mEncryptionTypes = {ENCRYPTION_RSA, ENCRYPTION_TOR_OBFS2, ENCRYPTION_ECC, ENCRYPTION_ECCXOR};
    std::random_shuffle(mEncryptionTypes.begin(), mEncryptionTypes.end());
    // Set encryption to type 0
    mCurrentEncryptionType = 0;
    Setting::instance()->setEncryption(mEncryptionTypes.at(mCurrentEncryptionType));
    // Get all servers
    mServerIds = AuthManager::instance()->currentEncryptionServers();
    std::random_shuffle(mServerIds.begin(), mServerIds.end());
    // Set server to first
    mCurrentServerId = -1;
    nextServer();
    Setting::instance()->setServer(mServerIds.at(mCurrentServerId));
    // Get all protocols
    mProtocols = Setting::instance()->currentEncryptionProtocols();
    mProtocolIds.clear();
    if (mQuickTest) {
        for (int i = 0; i < 4; ++i)
            mProtocolIds.append(i);
    } else {
        for (int i = 0; i < mProtocols.size(); ++i)
            mProtocolIds.append(i);
    }

    std::random_shuffle(mProtocolIds.begin(), mProtocolIds.end());
    // Set protocol to first
    resetPort();
    Setting::instance()->setProtocol(mProtocolIds.at(mCurrentProtocol));
    // Connect
    OpenvpnManager::instance()->start();
}

bool TestDialog::exists()
{
    return (mInstance.get() != NULL);
}

void TestDialog::cleanup()
{
    if (mInstance.get() != NULL)
        delete mInstance.release();
}

void TestDialog::setNoServer()
{
    ui->L_Percent->hide();
    ui->L_Percent->setText("0%");
    ui->L_LOAD->hide();
    ui->countryLabel->setText("No location specified.");
}

void TestDialog::setServer(int srv)
{
    if (srv < 0) {	// none
        setNoServer();
    } else {
        const AServer & se = AuthManager::instance()->getServer(srv);
        ui->countryLabel->setText(se.name);

        QString nip = AuthManager::instance()->newIP();
        if (nip.isEmpty())
            nip = se.address;

        double d = se.load.toDouble();
        int i = se.load.toInt();
        if (i == 0 && se.load != "0")
            i = (int)d;
        ui->L_Percent->setText(QString::number(i) + "%");
        ui->L_Percent->show();
        ui->L_LOAD->show();
    }
}

void TestDialog::updateServer()
{
    setServer(Setting::instance()->serverID());
}

void TestDialog::updateEncryption()
{
    int enc = Setting::instance()->encryption();
    ui->encryptionLabel->setText(Setting::encryptionName(enc));
}

void TestDialog::setAccountName(const QString & s)
{
    if (ui->L_Login->text().isEmpty() || ui->L_Login->text() == "--")
        ui->L_Login->setText(s);
    ui->L_Login->show();
}

void TestDialog::setEmail(const QString & s)
{
    ui->L_Email->setText(s);
    ui->L_Email->show();
}

void TestDialog::setAmount(const QString & s)
{
    ui->L_Amount->setText(s);
    ui->L_Amount->show();
}

void TestDialog::setUntil(const QString & date)
{
    ui->L_Until->setText("active until\n" + date);
    ui->L_Until->show();
}

void TestDialog::setProtocol(int ix)
{
    if (ix < 0)
        ui->L_Protocol->setText("Not selected");
    else
        ui->L_Protocol->setText(Setting::instance()->protocolName(ix));
}

void TestDialog::updateProtocol()
{
    setProtocol(Setting::instance()->currentProtocol());
}

void TestDialog::iterate(bool skipPorts)
{
    // First of all if we are not in quick test mode and not skipping because of error,
    // try the next protocol, otherwise go to the next server
    if (!mQuickTest && !skipPorts && ++mCurrentProtocol < mProtocols.size()) {
        // First see if we can just go to the next protocol
        Setting::instance()->setProtocol(mProtocolIds.at(mCurrentProtocol));
        OpenvpnManager::instance()->start();
        return;
    }
    nextServer();
    if (mCurrentServerId < mServerIds.size()) {
        // Got to the end of the list of protocols, so go to the next server
        // and start over at the top of the list of protocols
        const AServer & se = AuthManager::instance()->getServer(mServerIds.at(mCurrentServerId));
        qDebug() << "Switching to server " << mServerIds.at(mCurrentServerId)
                 << " which is " << se.name
                 << " address: " << se.address;
        Setting::instance()->setServer(mServerIds.at(mCurrentServerId));
        resetPort();
        Setting::instance()->setProtocol(mProtocolIds.at(mCurrentProtocol));
        OpenvpnManager::instance()->start();
        return;
    }
    // Got to the end of the list of servers, so switch to the next encryption
    // type and start over only if not on quick test mode
    if (!mQuickTest && ++mCurrentEncryptionType < mEncryptionTypes.size()) {
        Setting::instance()->setEncryption(mEncryptionTypes.at(mCurrentEncryptionType));
        // Get all servers
        mServerIds = AuthManager::instance()->currentEncryptionServers();

        std::random_shuffle(mServerIds.begin(), mServerIds.end());
        // Set server to first
        mCurrentServerId = 0;
        Setting::instance()->setServer(mServerIds.at(mCurrentServerId));
        // Get all protocols
        mProtocols = Setting::instance()->currentEncryptionProtocols();
        mProtocolIds.clear();
        for (int i = 0; i < mProtocols.size(); ++i)
            mProtocolIds.append(i);

        std::random_shuffle(mProtocolIds.begin(), mProtocolIds.end());

        // Set protocol to first
        resetPort();
        Setting::instance()->setProtocol(mProtocolIds.at(mCurrentProtocol));
        OpenvpnManager::instance()->start();
        return;
    }

    // Otherwise we finished checking all servers, all encryption types, all ports
    // Ask to save table widget to pipe separated values file.
    ui->saveCSVButton->setEnabled(true);
    ui->pauseButton->hide();
    ui->quickTestButton->show();
    ui->startButton->setText(kResetText);
}

int TestDialog::randomTCPPort()
{
    // give a port index between 0 and 3
    return (qrand() % 4);
}

int TestDialog::randomUDPPort()
{
    // give a port index between 4 and 8
    return (qrand() % 5) + 4;
}

void TestDialog::resetPort()
{
    if (mQuickTest)
        mCurrentProtocol = randomTCPPort();
    else
        mCurrentProtocol = 0;
}

void TestDialog::nextServer()
{
    AServer server;
    do {
        ++mCurrentServerId;
        server = AuthManager::instance()->getServer(mServerIds.at(mCurrentServerId));
    } while (mCurrentServerId < mServerIds.size() &&
             server.name.contains("Hub"));
}

int TestDialog::addRow()
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(row + 1);

    const AServer &server = AuthManager::instance()->getServer(mServerIds.at(mCurrentServerId));
    QTableWidgetItem *serverItem = new QTableWidgetItem(ui->countryLabel->text());
    serverItem->setToolTip(server.address);
    ui->tableWidget->setItem(row, kServerColumn, serverItem);
    QTableWidgetItem *encryptionItem = new QTableWidgetItem(ui->encryptionLabel->text());
    ui->tableWidget->setItem(row, kEncryptionColumn, encryptionItem);
    QTableWidgetItem *portItem = new QTableWidgetItem(Setting::instance()->port());
    ui->tableWidget->setItem(row, kPortColumn, portItem);
    return row;
}

void TestDialog::addConnected()
{
    int row = addRow();
    QTableWidgetItem *resultItem = new QTableWidgetItem(tr("success"));
    resultItem->setForeground(Qt::darkGreen);
    resultItem->setToolTip(AuthManager::instance()->newIP());
    ui->tableWidget->setItem(row, kResultColumn, resultItem);
}

void TestDialog::addError(QString message)
{
    int row = addRow();
    QTableWidgetItem *resultItem = new QTableWidgetItem(QString("failure %1").arg(message));
    resultItem->setForeground(Qt::red);
    ui->tableWidget->setItem(row, kResultColumn, resultItem);
}

bool TestDialog::saveCSV(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        // Show file error dialog
        return false;
    }

    // Create a folder with the same name as the filename but with .logs extension
    QString logfolder = filename + ".logs";
    QDir logDir(logfolder);

    if (!logDir.exists())
        logDir.mkdir(logfolder);
    else if (!logDir.entryList(QDir::NoDotAndDotDot).isEmpty()) {
        // Clean out the log folder
        QStringList filenames = logDir.entryList(QDir::NoDotAndDotDot);
        foreach(QString filename, filenames)
            logDir.remove(filename);
    }

    // Iterate over the rows of the table
    for (int i=0; i < ui->tableWidget->rowCount(); ++i) {
        // Write server name and ip address from the widget item
        QString serverName = ui->tableWidget->item(i, kServerColumn)->text();
        QString encryptionName = ui->tableWidget->item(i, kEncryptionColumn)->text();
        QString protocolName = ui->tableWidget->item(i, kPortColumn)->text();
        file.write(serverName.toLatin1());
        file.write("|");
        file.write(ui->tableWidget->item(i, kServerColumn)->toolTip().toLatin1());
        file.write("|");
        // Write the port number
        file.write(protocolName.toLatin1());
        file.write("|");
        // Write the encryption type
        file.write(encryptionName.toLatin1());
        file.write("|");
        // Write the result
        file.write(ui->tableWidget->item(i, kResultColumn)->text().toLatin1());
        file.write("|");
        // Write the debug file name if it failed
        QString newIp = ui->tableWidget->item(i, kResultColumn)->toolTip();
        if (newIp.isEmpty()) {
            // Copy logs
            QString logFilename = QString("%1-%2-%3-%4").arg(serverName)
                                  .arg(encryptionName).arg(protocolName).arg("openvpn.log");
            QString debugFilename = QString("%1-%2-%3-%4").arg(serverName)
                                    .arg(encryptionName).arg(protocolName).arg("debug.log");
            QFile::rename(mLogFolder + "/" + logFilename, logfolder + "/" + logFilename);
            QFile::rename(mLogFolder + "/" + debugFilename, logfolder + "/" + debugFilename);

            // Failure, write the debug and log filenames (and copy the files)
            file.write(logfolder.toLatin1());
            file.write("/");
            file.write(debugFilename.toLatin1());
            file.write("|");
            // Write the log file name if it failed
            file.write(logfolder.toLatin1());
            file.write("/");
            file.write(logFilename.toLatin1());
            file.write("|");
            // Write null for the new ip address because it's a failure
            file.write("null");
        } else {
            // Success, write null for filenames and write new ip
            file.write("null");
            file.write("|");
            // Write the log file name if it failed
            file.write("null");
            file.write("|");
            // Write the new ip address
            file.write(newIp.toLatin1());
        }
        // Write a newline
        file.write("\n");
    }
    file.close();
    return true;
}

TestDialog::~TestDialog()
{
    {
        if (this->isVisible()) {
            WndManager::Instance()->HideThis(this);
            WndManager::Instance()->SavePos();
        }
    }
    delete ui;
}

void TestDialog::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

static const char * gs_ConnGreen = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-green.png);\n}";
static const char * gs_ConnRed = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-red.png);\n}";
static const char * gs_Conn_Connecting = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-yellow.png);\n}";
static const char * gs_Conn_Connecting_Template_start = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-y-";
static const char * gs_Conn_Connecting_Template_end =  ".png);\n}";

void TestDialog::initializeStateWords()
{
    if (mStateWordImages.empty()) {
        mStateWordImages.insert("AUTH", "auth");
        mStateWordImages.insert("GET_CONFIG", "config");
        mStateWordImages.insert("ASSIGN_IP", "ip");
        mStateWordImages.insert("TCP_CONNECT", "connect");
        mStateWordImages.insert("RESOLVE", "resolve");

        // CONNECTING - default - must be absent in this collection

        mStateWordImages.insert("WAIT", "wait");
        mStateWordImages.insert("RECONNECTING", "reconn");
    }
}

void TestDialog::on_startButton_clicked()
{
    if (ui->startButton->text() == kStartText) {
        mQuickTest = false;
        startTest();
    } else { // Reset
        ui->pauseButton->hide();
        ui->quickTestButton->show();
        ui->startButton->setText(kStartText);
        ui->saveCSVButton->setEnabled(true);

        OpenvpnManager::instance()->stop();
        setStatusDisconnected();
    }
}

void TestDialog::on_pauseButton_clicked()
{
    if (ui->pauseButton->text().compare(kPauseText) == 0) {
        ui->pauseButton->setText(kResumeText);
        ui->saveCSVButton->setEnabled(true);
        OpenvpnManager::instance()->stop();
    } else {
        ui->pauseButton->setText(kPauseText);
        ui->saveCSVButton->setEnabled(false);
        OpenvpnManager::instance()->start();
    }
}

void TestDialog::on_saveCSVButton_clicked()
{
    // Get filename
    QSettings settings;
    QString filename = QFileDialog::getSaveFileName(this,
                       "Save CSV file",
                       settings.value(kLastFilenameKey, QDir::home().absolutePath()).toString(),
                       "CSV File (*.csv)");
    // Save table to file
    if (saveCSV(filename))
        WndManager::Instance()->Confirmation("CSV File Saved");
    else
        WndManager::Instance()->ErrMsg(QString("Unable to save to file %1").arg(filename));
}

void TestDialog::setStatusConnecting()
{
    ui->L_ConnectStatus->setStyleSheet(gs_Conn_Connecting);
}

void TestDialog::setStatusConnecting(const QString & word)
{
    initializeStateWords();

    QString s;
    QHash<QString, const char*>::iterator it = mStateWordImages.find(word);
    if (it != mStateWordImages.end()) {
        s = gs_Conn_Connecting_Template_start;
        s += it.value();
        s += gs_Conn_Connecting_Template_end;
    } else {
        s = gs_Conn_Connecting;
        if (word.compare("WAIT", Qt::CaseInsensitive) == 0) {
            log::logt("Cannot find WAIT in the collection! Do actions manualy!");
            s = gs_Conn_Connecting_Template_start;
            s += "wait";
            s += gs_Conn_Connecting_Template_end;
        }
    }
    ui->L_ConnectStatus->setStyleSheet(s);
}

void TestDialog::setStatusConnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnGreen);
    // Add the current encryption type, server, and port to the table
    addConnected();

    OpenvpnManager::instance()->stop();
    iterate(false);
}

void TestDialog::setStatusDisconnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnRed);
}

void TestDialog::setError(const QString &message)
{
    OpenvpnManager::instance()->stop();

    addError(message);

    // Copy logs
    QString serverName = ui->countryLabel->text();
    QString encryptionName = ui->encryptionLabel->text();
    QString protocolName = Setting::instance()->port();
    QFile::copy(PathHelper::Instance()->openvpnLogFilename(),
                QString("%1/%2-%3-%4-%5").arg(mLogFolder).arg(serverName).arg(encryptionName)
                .arg(protocolName).arg("openvpn.log"));
    QFile::copy(PathHelper::Instance()->safejumperLogFilename(),
                QString("%1/%2-%3-%4-%5").arg(mLogFolder).arg(serverName).arg(encryptionName)
                .arg(protocolName).arg("debug.log"));
    iterate(true);
}

TestDialog * TestDialog::instance()
{
    if (!mInstance.get()) {
        mInstance.reset(new TestDialog());
    }
    return mInstance.get();
}

void TestDialog::keyPressEvent(QKeyEvent * e)
{
    if (e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}

void TestDialog::on_quickTestButton_clicked()
{
    mQuickTest = true;
    startTest();
}

void TestDialog::PortDlgAction(int action)
{
    if (QDialog::Accepted == action) {
        OpenvpnManager::instance()->startPortLoop(WndManager::Instance()->IsCyclePort());
    }
}
