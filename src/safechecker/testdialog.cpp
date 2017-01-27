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

TestDialog::HmWords TestDialog::_StateWord_Img;

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog)
    , _moving(false)
{
    ui->setupUi(this);

    ui->countryLabel->setText("");
    ui->L_Percent->setText("0%");
    ui->L_OldIp->setText("");
    ui->L_NewIp->setText("");
    SetNoSrv();

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

    StatusDisconnected();

    ui->cancelButton->hide();

    ui->L_Until->setText("active until\n-");
    ui->L_Amount->setText("-");
    ui->L_OldIp->setText("");

//	QPoint p0 = _WndStart = pos();
//	WndManager::DoShape(this);
//	QPoint p1 = pos();
//	if (p0 != p1)
//	{
//		log::logt("Non equal! Move back;");
//		move(p0);
//	}
    qApp->installEventFilter(this);
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

bool TestDialog::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseMove: {
        if (_moving) {
            QPoint d = QCursor::pos() - _CursorStart;
            if (d.x() != 0 || d.y() != 0) {
                QPoint NewAbs = _WndStart + d;
                this->move(NewAbs);
            }
        }
        return false;
    }
    case QEvent::MouseButtonRelease: {
        _moving = false;
//			_WndStart = pos();
        return false;
    }
    default:
        return QDialog::eventFilter(obj, event);
    }
}

void TestDialog::Init()
{
    // Setting::Instance()->LoadServer();
    Setting::Instance()->LoadProt();

    // TODO: -1  get actual data
    ui->L_Until->setText("active until\n-");
    ui->L_Amount->setText("-");
    SetOldIp(AuthManager::Instance()->OldIp());
    UpdEnc();
    UpdProtocol();
}

void TestDialog::SetNoSrv()
{
    ui->L_Percent->hide();
    ui->L_Percent->setText("0%");
    ui->L_LOAD->hide();
    ui->flagButton->hide();
    ui->L_NewIp->hide();
    ui->countryLabel->setText("No location specified.");
}

void TestDialog::SetServer(int srv)
{
    if (srv < 0) {	// none
        SetNoSrv();
    } else {
        const AServer & se = AuthManager::Instance()->GetSrv(srv);
        ui->countryLabel->setText(se.name);
        ui->flagButton->show();

        QString nip = AuthManager::Instance()->NewIp();
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
        SetFlag(srv);
    }
}

void TestDialog::DwnlStrs()
{

}

void TestDialog::UpdNewIp(const QString & s)
{
    static const QString self = "127.0.0.1";
    if (s != self) {
        ui->L_NewIp->setText(s);
        ui->L_NewIp->show();
    }
}

void TestDialog::UpdEnc()
{
    int enc = Setting::Encryption();
    ui->encryptionLabel->setText(Setting::EncText(enc));
}

void TestDialog::SetOldIp(const QString & s)
{
    ui->L_OldIp->setText(s);
    ui->L_OldIp->show();
}

void TestDialog::SetAccName(const QString & s)
{
    if (ui->L_Login->text().isEmpty() || ui->L_Login->text() == "--")
        ui->L_Login->setText(s);
    ui->L_Login->show();
}

void TestDialog::SetEmail(const QString & s)
{
    ui->L_Email->setText(s);
    ui->L_Email->show();
}

void TestDialog::SetAmount(const QString & s)
{
    ui->L_Amount->setText(s);
    ui->L_Amount->show();
}

void TestDialog::SetUntil(const QString & date)
{
    ui->L_Until->setText("active until\n" + date);
    ui->L_Until->show();
}

void TestDialog::SetFlag(int srv)
{
    QString n = AuthManager::Instance()->GetSrv(srv).name;
    QString fl = flag::IconFromSrvName(n);
    ui->flagButton->setStyleSheet("QPushButton\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/flags/" + fl + ".png);\n}");
}

void TestDialog::SetProtocol(int ix)
{
    if (ix < 0)
        ui->L_Protocol->setText("Not selected");
    else
        ui->L_Protocol->setText(Setting::Instance()->ProtoStr(ix));
}

void TestDialog::UpdProtocol()
{
    SetProtocol(Setting::Instance()->CurrProto());
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

void TestDialog::InitStateWords()
{
    if (_StateWord_Img.empty()) {
        _StateWord_Img.insert("AUTH", "auth");
        _StateWord_Img.insert("GET_CONFIG", "config");
        _StateWord_Img.insert("ASSIGN_IP", "ip");
        _StateWord_Img.insert("TCP_CONNECT", "connect");
        _StateWord_Img.insert("RESOLVE", "resolve");

        // CONNECTING - default - must be absent in this collection

        _StateWord_Img.insert("WAIT", "wait");
        _StateWord_Img.insert("RECONNECTING", "reconn");
    }
}

void TestDialog::on_startButton_clicked()
{
    // Get all encryption types
    mEncryptionTypes = {ENCRYPTION_RSA, ENCRYPTION_OBFS_TOR, ENCRYPTION_ECC, ENCRYPTION_ECCXOR};
    // Set encryption to type 0
    mCurrentEncryptionType = 0;
    // Get all servers
    mServerIds = AuthManager::Instance()->currentEncryptionServers();
    // Set server to first
    mCurrentServerId = 0;
    // Get all protocols
    mProtocols = Setting::Instance()->GetAllPorts();
    // Set protocol to first
    // Get all ports
    // Set port to first
    // Connect
}

void TestDialog::on_cancelButton_clicked()
{

}

void TestDialog::StatusConnecting()
{
    ui->L_ConnectStatus->setStyleSheet(gs_Conn_Connecting);
    SetEnabledButtons(false);
}

void TestDialog::StatusConnecting(const QString & word)
{
//	this->StatusConnecting();
    SetEnabledButtons(false);
    InitStateWords();

    QString s;
    HmWords::iterator it = _StateWord_Img.find(word);
    if (it != _StateWord_Img.end()) {
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

void TestDialog::SetEnabledButtons(bool enabled)
{
    if (enabled) {
        ui->startButton->show();
        ui->cancelButton->hide();
    } else {
        ui->startButton->hide();
        ui->cancelButton->show();
    }

    ui->flagButton->setEnabled(enabled);
}

void TestDialog::StatusConnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnGreen);
    SetEnabledButtons(true);
    ui->startButton->hide();
    ui->cancelButton->show();
}

void TestDialog::StatusDisconnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnRed);
    SetEnabledButtons(true);
}

std::auto_ptr<TestDialog> TestDialog::mInstance;
TestDialog * TestDialog::instance()
{
    if (!mInstance.get()) {
        mInstance.reset(new TestDialog());
        mInstance->Init();
    }
    return mInstance.get();
}

void TestDialog::keyPressEvent(QKeyEvent * e)
{
    if (e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}

void TestDialog::PortDlgAction(int action)
{
    if (QDialog::Accepted == action) {
        OpenvpnManager::Instance()->startPortLoop(WndManager::Instance()->IsCyclePort());
    }
}





