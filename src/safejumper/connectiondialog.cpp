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

ConnectionDialog::HmWords ConnectionDialog::_StateWord_Img;

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
    , _moving(false)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    ui->L_Country->setText("");
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

    ui->b_Cancel->hide();

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

    connect(AuthManager::Instance(), SIGNAL(oldIpLoaded(QString)),
            this, SLOT(SetOldIp(QString)));
    connect(AuthManager::Instance(), SIGNAL(emailLoaded(QString)),
            this, SLOT(SetEmail(QString)));
    connect(AuthManager::Instance(), SIGNAL(untilLoaded(QString)),
            this, SLOT(SetUntil(QString)));
    connect(AuthManager::Instance(), SIGNAL(amountLoaded(QString)),
            this, SLOT(SetAmount(QString)));
}

bool ConnectionDialog::eventFilter(QObject *obj, QEvent *event)
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

void ConnectionDialog::Init()
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

void ConnectionDialog::SetNoSrv()
{
    ui->L_Percent->hide();
    ui->L_Percent->setText("0%");
    ui->L_LOAD->hide();
    ui->b_Flag->hide();
    ui->L_NewIp->hide();
    ui->L_Country->setText("No location specified.");
}

void ConnectionDialog::SetServer(int srv)
{
    if (srv < 0) {	// none
        SetNoSrv();
    } else {
        const AServer & se = AuthManager::Instance()->GetSrv(srv);
        ui->L_Country->setText(se.name);
        ui->b_Flag->show();
        ui->b_FlagBox->show();

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

void ConnectionDialog::DwnlStrs()
{

}

void ConnectionDialog::UpdNewIp(const QString & s)
{
    static const QString self = "127.0.0.1";
    if (s != self) {
        ui->L_NewIp->setText(s);
        ui->L_NewIp->show();
    }
}

void ConnectionDialog::UpdEnc()
{
    int enc = Setting::Encryption();
    ui->L_Encryption->setText(Setting::EncText(enc));
}

void ConnectionDialog::SetOldIp(const QString & s)
{
    ui->L_OldIp->setText(s);
    ui->L_OldIp->show();
}

void ConnectionDialog::SetAccName(const QString & s)
{
    if (ui->L_Login->text().isEmpty() || ui->L_Login->text() == "--")
        ui->L_Login->setText(s);
    ui->L_Login->show();
}

void ConnectionDialog::SetEmail(const QString & s)
{
    ui->L_Email->setText(s);
    ui->L_Email->show();
}

void ConnectionDialog::SetAmount(const QString & s)
{
    ui->L_Amount->setText(s);
    ui->L_Amount->show();
}

void ConnectionDialog::SetUntil(const QString & date)
{
    ui->L_Until->setText("active until\n" + date);
    ui->L_Until->show();
}

void ConnectionDialog::SetFlag(int srv)
{
    QString n = AuthManager::Instance()->GetSrv(srv).name;
    QString fl = flag::IconFromSrvName(n);
    ui->b_Flag->setStyleSheet("QPushButton\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/flags/" + fl + ".png);\n}");
}

void ConnectionDialog::SetProtocol(int ix)
{
    if (ix < 0)
        ui->L_Protocol->setText("Not selected");
    else
        ui->L_Protocol->setText(Setting::Instance()->ProtoStr(ix));
}

void ConnectionDialog::UpdProtocol()
{
    SetProtocol(Setting::Instance()->CurrProto());
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

void ConnectionDialog::InitStateWords()
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

void ConnectionDialog::StatusConnecting()
{
    ui->L_ConnectStatus->setStyleSheet(gs_Conn_Connecting);
    SetEnabledButtons(false);
}

void ConnectionDialog::StatusConnecting(const QString & word)
{
//	ModifyWndTitle(word);
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

void ConnectionDialog::ModifyWndTitle(const QString & word)
{
    QString s = "Safejumper";
//	if (!word.isEmpty())
//		s += " " + word;
    this->setWindowTitle(s);
}

void ConnectionDialog::SetEnabledButtons(bool enabled)
{
    if (enabled) {
        ui->b_Connect->show();
        ui->b_Cancel->hide();
    } else {
        ui->b_Connect->hide();
        ui->b_Cancel->show();
    }

    ui->b_Flag->setEnabled(enabled);
    ui->b_FlagBox->setEnabled(enabled);
    ui->b_Row_Country->setEnabled(enabled);
    ui->b_Row_Ip->setEnabled(enabled);
    ui->b_Row_Protocol->setEnabled(enabled);
}

void ConnectionDialog::StatusConnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnGreen);
    SetEnabledButtons(true);
    ui->b_Connect->hide();
    ui->b_Cancel->show();
    ModifyWndTitle("");
}

void ConnectionDialog::StatusDisconnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnRed);
    SetEnabledButtons(true);
    ModifyWndTitle("");
}

std::auto_ptr<ConnectionDialog> ConnectionDialog::_inst;
ConnectionDialog * ConnectionDialog::Instance()
{
    if (!_inst.get()) {
        _inst.reset(new ConnectionDialog());
        _inst->Init();
    }
    return _inst.get();
}

void ConnectionDialog::ToScr_Settings()
{
    WndManager::Instance()->ToSettings();
}

void ConnectionDialog::ToScr_Primary()
{
    WndManager::Instance()->ToPrimary();
}

void ConnectionDialog::ToScr_Login()
{
    WndManager::Instance()->ToPrimary();
}

void ConnectionDialog::ToScr_Map()
{
    WndManager::Instance()->ToMap();
}

void ConnectionDialog::ShowPackageUrl()
{
    OpenUrl_Panel();
}

void ConnectionDialog::Clicked_Connect()
{
    OpenvpnManager::Instance()->start();		// handle visuals inside
}

void ConnectionDialog::Clicked_Cancel()
{
#ifdef MONITOR_TOOL
    Ctr_Openvpn::Instance()->StopLoop();
#endif	// MONITOR_TOOL
    OpenvpnManager::Instance()->stop();
    LoginWindow::Instance()->BlockOnDisconnect();
}

void ConnectionDialog::Clicked_Jump()
{
    AuthManager::Instance()->jump();
}

void ConnectionDialog::Clicked_Min()
{
    WndManager::Instance()->HideThis(this);
}

void ConnectionDialog::Clicked_Cross()
{
    LoginWindow::Instance()->quitApplication();
}

void ConnectionDialog::ConnectError(QProcess::ProcessError error)
{
    log::logt("Scr_Connect::ConnectError(): error = " + QString::number(error));
    WndManager::Instance()->HandleDisconnected();
}

void ConnectionDialog::ConnectStarted()
{
    log::logt("Scr_Connect::ConnectStarted()");
}

void ConnectionDialog::ConnectStateChanged(QProcess::ProcessState newState)
{
    log::logt("Scr_Connect::ConnectStateChanged(): newState = " + QString::number(newState));
    OpenvpnManager::Instance()->processStateChanged(newState);
}

void ConnectionDialog::ConnectFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    log::logt("Scr_Connect::ConnectFinished(): exitCode = " + QString::number(exitCode) + " exitStatus = " +  QString::number(exitStatus));
    OpenvpnManager::Instance()->processFinished(exitCode, exitStatus);
}

void ConnectionDialog::ConnectStderr()
{
    OpenvpnManager::Instance()->logStderr();
}

void ConnectionDialog::ConnectStdout()
{
    OpenvpnManager::Instance()->logStdout();
}

void ConnectionDialog::Pressed_Head()
{
    _WndStart = this->pos();
    _CursorStart = QCursor::pos();
    _moving = true;
}

void ConnectionDialog::keyPressEvent(QKeyEvent * e)
{
    if(e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}

void ConnectionDialog::PortDlgAction(int action)
{
    if (QDialog::Accepted == action) {
        OpenvpnManager::Instance()->startPortLoop(WndManager::Instance()->IsCyclePort());
    }
}





