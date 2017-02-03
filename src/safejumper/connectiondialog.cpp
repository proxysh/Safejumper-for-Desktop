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

#include "connectiondialog.h"

#include "ui_connectiondialog.h"
#include "scr_settings.h"
#include "scr_map.h"
#include "loginwindow.h"
#include "authmanager.h"
#include "common.h"
#include "wndmanager.h"
#include "setting.h"
#include "openvpnmanager.h"
#include "log.h"
#include "flag.h"
#include "fonthelper.h"

ConnectionDialog::HmWords ConnectionDialog::mStateWordImages;

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    ui->L_Country->setText("");
    ui->L_Percent->setText("0%");
    ui->L_OldIp->setText("");
    ui->L_NewIp->setText("");
    setNoServer();

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
    QPoint p1 = ui->L_LOAD->pos();
    p1.setX(p1.x() + 10);
    ui->L_LOAD->move(p1);
#endif

    statusDisconnected();

    ui->cancelButton->hide();

    ui->L_Until->setText("active until\n-");
    ui->L_Amount->setText("-");
    ui->L_OldIp->setText("");

    connect(AuthManager::instance(), SIGNAL(oldIpLoaded(QString)),
            this, SLOT(setOldIP(QString)));
    connect(AuthManager::instance(), SIGNAL(emailLoaded(QString)),
            this, SLOT(setEmail(QString)));
    connect(AuthManager::instance(), SIGNAL(untilLoaded(QString)),
            this, SLOT(setUntil(QString)));
    connect(AuthManager::instance(), SIGNAL(amountLoaded(QString)),
            this, SLOT(setAmount(QString)));

    // Setting::Instance()->LoadServer();
    Setting::instance()->loadProtocol();

    // TODO: -1  get actual data
    ui->L_Until->setText("active until\n-");
    ui->L_Amount->setText("-");
    setOldIP(AuthManager::instance()->oldIP());
    updateEncoding();
    updateProtocol();
}

bool ConnectionDialog::exists()
{
    return (mInstance.get() != NULL);
}

void ConnectionDialog::cleanup()
{
    if (mInstance.get() != NULL)
        delete mInstance.release();
}

void ConnectionDialog::setNoServer()
{
    ui->L_Percent->hide();
    ui->L_Percent->setText("0%");
    ui->L_LOAD->hide();
    ui->b_Flag->hide();
    ui->L_NewIp->hide();
    ui->L_Country->setText("No location specified.");
}

void ConnectionDialog::setServer(int srv)
{
    if (srv < 0) {	// none
        setNoServer();
    } else {
        const AServer & se = AuthManager::instance()->getServer(srv);
        ui->L_Country->setText(se.name);
        ui->b_Flag->show();
        ui->b_FlagBox->show();

        QString nip = AuthManager::instance()->newIP();
        if (nip.isEmpty())
            nip = se.address;
        ui->L_NewIp->setText(nip);
        ui->L_NewIp->show();

        double d = se.load.toDouble();
        int i = se.load.toInt();
        if (i == 0 && se.load != "0")
            i = (int)d;
        ui->L_Percent->setText(QString::number(i) + "%");
        ui->L_Percent->show();
        ui->L_LOAD->show();
        setFlag(srv);
    }
}

void ConnectionDialog::updateNewIP(const QString & s)
{
    static const QString self = "127.0.0.1";
    if (s != self) {
        ui->L_NewIp->setText(s);
        ui->L_NewIp->show();
    }
}

void ConnectionDialog::updateEncoding()
{
    int enc = Setting::encryption();
    ui->L_Encryption->setText(Setting::encryptionName(enc));
}

void ConnectionDialog::setOldIP(const QString & s)
{
    ui->L_OldIp->setText(s);
    ui->L_OldIp->show();
}

void ConnectionDialog::setAccountName(const QString & s)
{
    if (ui->L_Login->text().isEmpty() || ui->L_Login->text() == "--")
        ui->L_Login->setText(s);
    ui->L_Login->show();
}

void ConnectionDialog::setEmail(const QString & s)
{
    ui->L_Email->setText(s);
    ui->L_Email->show();
}

void ConnectionDialog::setAmount(const QString & s)
{
    ui->L_Amount->setText(s);
    ui->L_Amount->show();
}

void ConnectionDialog::setUntil(const QString & date)
{
    ui->L_Until->setText("active until\n" + date);
    ui->L_Until->show();
}

void ConnectionDialog::setFlag(int srv)
{
    QString n = AuthManager::instance()->getServer(srv).name;
    QString fl = flag::IconFromSrvName(n);
    ui->b_Flag->setStyleSheet("QPushButton\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/flags/" + fl + ".png);\n}");
}

void ConnectionDialog::setProtocol(int ix)
{
    if (ix < 0)
        ui->L_Protocol->setText("Not selected");
    else
        ui->L_Protocol->setText(Setting::instance()->protocolName(ix));
}

void ConnectionDialog::updateProtocol()
{
    setProtocol(Setting::instance()->currentProtocol());
}

ConnectionDialog::~ConnectionDialog()
{
    {
        if (this->isVisible()) {
            WndManager::Instance()->HideThis(this);
            WndManager::Instance()->SavePos();
        }
    }
    delete ui;
}

void ConnectionDialog::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

static const char * gs_ConnGreen = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-green.png);\n}";
static const char * gs_ConnRed = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-red.png);\n}";
static const char * gs_Conn_Connecting = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-yellow.png);\n}";
static const char * gs_Conn_Connecting_Template_start = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-y-";
static const char * gs_Conn_Connecting_Template_end =  ".png);\n}";

void ConnectionDialog::initializeStateWords()
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

void ConnectionDialog::statusConnecting()
{
    ui->L_ConnectStatus->setStyleSheet(gs_Conn_Connecting);
    enableButtons(false);
}

void ConnectionDialog::statusConnecting(const QString & word)
{
//	ModifyWndTitle(word);
//	this->StatusConnecting();
    enableButtons(false);
    initializeStateWords();

    QString s;
    HmWords::iterator it = mStateWordImages.find(word);
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

void ConnectionDialog::enableButtons(bool enabled)
{
    if (enabled) {
        ui->connectButton->show();
        ui->cancelButton->hide();
    } else {
        ui->connectButton->hide();
        ui->cancelButton->show();
    }

    ui->b_Flag->setEnabled(enabled);
    ui->b_FlagBox->setEnabled(enabled);
    ui->b_Row_Country->setEnabled(enabled);
    ui->b_Row_Ip->setEnabled(enabled);
    ui->b_Row_Protocol->setEnabled(enabled);
}

void ConnectionDialog::statusConnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnGreen);
    enableButtons(true);
    ui->connectButton->hide();
    ui->cancelButton->show();
}

void ConnectionDialog::statusDisconnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnRed);
    enableButtons(true);
}

std::auto_ptr<ConnectionDialog> ConnectionDialog::mInstance;
ConnectionDialog * ConnectionDialog::instance()
{
    if (!mInstance.get()) {
        mInstance.reset(new ConnectionDialog());
    }
    return mInstance.get();
}

void ConnectionDialog::on_settingsButton_clicked()
{
    WndManager::Instance()->ToSettings();
}

void ConnectionDialog::showMainWindow()
{
    WndManager::Instance()->ToPrimary();
}

void ConnectionDialog::showLoginWindow()
{
    WndManager::Instance()->ToPrimary();
}

void ConnectionDialog::showMapWindow()
{
    WndManager::Instance()->ToMap();
}

void ConnectionDialog::showPackageUrl()
{
    OpenUrl_Panel();
}

void ConnectionDialog::on_connectButton_clicked()
{
    OpenvpnManager::instance()->start();		// handle visuals inside
}

void ConnectionDialog::on_cancelButton_clicked()
{
    OpenvpnManager::instance()->stop();
    LoginWindow::Instance()->BlockOnDisconnect();
}

void ConnectionDialog::on_jumpButton_clicked()
{
    AuthManager::instance()->jump();
}

void ConnectionDialog::keyPressEvent(QKeyEvent * e)
{
    if(e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}

void ConnectionDialog::portDialogResult(int action)
{
    if (QDialog::Accepted == action) {
        OpenvpnManager::instance()->startPortLoop(WndManager::Instance()->IsCyclePort());
    }
}





