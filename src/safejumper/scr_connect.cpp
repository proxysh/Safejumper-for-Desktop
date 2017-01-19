#include "scr_connect.h"

#include "ui_scr_connect.h"
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

Scr_Connect::HmWords Scr_Connect::_StateWord_Img;

Scr_Connect::Scr_Connect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Scr_Connect)
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
}

bool Scr_Connect::eventFilter(QObject *obj, QEvent *event)
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

void Scr_Connect::Init()
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

void Scr_Connect::SetNoSrv()
{
    ui->L_Percent->hide();
    ui->L_Percent->setText("0%");
    ui->L_LOAD->hide();
    ui->b_Flag->hide();
    ui->L_NewIp->hide();
    ui->L_Country->setText("No location specified.");
}

void Scr_Connect::SetServer(int srv)
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

void Scr_Connect::DwnlStrs()
{

}

void Scr_Connect::UpdNewIp(const QString & s)
{
    static const QString self = "127.0.0.1";
    if (s != self) {
        ui->L_NewIp->setText(s);
        ui->L_NewIp->show();
    }
}

void Scr_Connect::UpdEnc()
{
    int enc = Setting::Encryption();
    ui->L_Encryption->setText(Setting::EncText(enc));
}

void Scr_Connect::SetOldIp(const QString & s)
{
    ui->L_OldIp->setText(s);
    ui->L_OldIp->show();
}

void Scr_Connect::SetAccName(const QString & s)
{
    if (ui->L_Login->text().isEmpty() || ui->L_Login->text() == "--")
        ui->L_Login->setText(s);
    ui->L_Login->show();
}

void Scr_Connect::SetEmail(const QString & s)
{
    ui->L_Email->setText(s);
    ui->L_Email->show();
}

void Scr_Connect::SetAmount(const QString & s)
{
    ui->L_Amount->setText(s);
    ui->L_Amount->show();
}

void Scr_Connect::SetUntil(const QString & date)
{
    ui->L_Until->setText("active until\n" + date);
    ui->L_Until->show();
}

void Scr_Connect::SetFlag(int srv)
{
    QString n = AuthManager::Instance()->GetSrv(srv).name;
    QString fl = flag::IconFromSrvName(n);
    ui->b_Flag->setStyleSheet("QPushButton\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/flags/" + fl + ".png);\n}");
}

void Scr_Connect::SetProtocol(int ix)
{
    if (ix < 0)
        ui->L_Protocol->setText("Not selected");
    else
        ui->L_Protocol->setText(Setting::Instance()->ProtoStr(ix));
}

void Scr_Connect::UpdProtocol()
{
    SetProtocol(Setting::Instance()->CurrProto());
}

Scr_Connect::~Scr_Connect()
{
    {
        if (this->isVisible()) {
            WndManager::Instance()->HideThis(this);
            WndManager::Instance()->SavePos();
        }
    }
    delete ui;
}

void Scr_Connect::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

static const char * gs_ConnGreen = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-green.png);\n}";
static const char * gs_ConnRed = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-red.png);\n}";
static const char * gs_Conn_Connecting = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-yellow.png);\n}";
static const char * gs_Conn_Connecting_Template_start = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-y-";
static const char * gs_Conn_Connecting_Template_end =  ".png);\n}";

void Scr_Connect::InitStateWords()
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

void Scr_Connect::StatusConnecting()
{
    ui->L_ConnectStatus->setStyleSheet(gs_Conn_Connecting);
    SetEnabledButtons(false);
}

void Scr_Connect::StatusConnecting(const QString & word)
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

void Scr_Connect::ModifyWndTitle(const QString & word)
{
    QString s = "Safejumper";
//	if (!word.isEmpty())
//		s += " " + word;
    this->setWindowTitle(s);
}

void Scr_Connect::SetEnabledButtons(bool enabled)
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

void Scr_Connect::StatusConnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnGreen);
    SetEnabledButtons(true);
    ui->b_Connect->hide();
    ui->b_Cancel->show();
    ModifyWndTitle("");
}

void Scr_Connect::StatusDisconnected()
{
    ui->L_ConnectStatus->setStyleSheet(gs_ConnRed);
    SetEnabledButtons(true);
    ModifyWndTitle("");
}

std::auto_ptr<Scr_Connect> Scr_Connect::_inst;
Scr_Connect * Scr_Connect::Instance()
{
    if (!_inst.get()) {
        _inst.reset(new Scr_Connect());
        _inst->Init();
    }
    return _inst.get();
}

void Scr_Connect::ToScr_Settings()
{
    WndManager::Instance()->ToSettings();
}

void Scr_Connect::ToScr_Primary()
{
    WndManager::Instance()->ToPrimary();
}

void Scr_Connect::ToScr_Login()
{
    WndManager::Instance()->ToPrimary();
}

void Scr_Connect::ToScr_Map()
{
    WndManager::Instance()->ToMap();
}

void Scr_Connect::ShowPackageUrl()
{
    OpenUrl_Panel();
}

void Scr_Connect::Clicked_Connect()
{
    OpenvpnManager::Instance()->start();		// handle visuals inside
}

void Scr_Connect::Clicked_Cancel()
{
#ifdef MONITOR_TOOL
    Ctr_Openvpn::Instance()->StopLoop();
#endif	// MONITOR_TOOL
    OpenvpnManager::Instance()->stop();
    LoginWindow::Instance()->BlockOnDisconnect();
}

void Scr_Connect::Clicked_Jump()
{
    AuthManager::Instance()->jump();
}

void Scr_Connect::Clicked_Min()
{
    WndManager::Instance()->HideThis(this);
}

void Scr_Connect::Clicked_Cross()
{
    LoginWindow::Instance()->quitApplication();
}

void Scr_Connect::ConnectError(QProcess::ProcessError error)
{
    log::logt("Scr_Connect::ConnectError(): error = " + QString::number(error));
    WndManager::Instance()->HandleDisconnected();
}

void Scr_Connect::ConnectStarted()
{
    log::logt("Scr_Connect::ConnectStarted()");
}

void Scr_Connect::ConnectStateChanged(QProcess::ProcessState newState)
{
    log::logt("Scr_Connect::ConnectStateChanged(): newState = " + QString::number(newState));
    OpenvpnManager::Instance()->processStateChanged(newState);
}

void Scr_Connect::ConnectFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    log::logt("Scr_Connect::ConnectFinished(): exitCode = " + QString::number(exitCode) + " exitStatus = " +  QString::number(exitStatus));
    OpenvpnManager::Instance()->processFinished(exitCode, exitStatus);
}

void Scr_Connect::ConnectStderr()
{
    OpenvpnManager::Instance()->logStderr();
}

void Scr_Connect::ConnectStdout()
{
    OpenvpnManager::Instance()->logStdout();
}

void Scr_Connect::Pressed_Head()
{
    _WndStart = this->pos();
    _CursorStart = QCursor::pos();
    _moving = true;
}

void Scr_Connect::keyPressEvent(QKeyEvent * e)
{
    if(e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}

void Scr_Connect::PortDlgAction(int action)
{
    if (QDialog::Accepted == action) {
        OpenvpnManager::Instance()->startPortLoop(WndManager::Instance()->IsCyclePort());
    }
}





