#include "scr_connect.h"

#include "ui_scr_connect.h"
#include "scr_settings.h"
#include "scr_map.h"
#include "sjmainwindow.h"
#include "authmanager.h"
#include "common.h"
#include "wndmanager.h"
#include "setting.h"
#include "ctr_openvpn.h"
#include "log.h"
#include "flag.h"
#include "fonthelper.h"

Scr_Connect::Scr_Connect(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Scr_Connect)
  , _moving(false)
{
	ui->setupUi(this);
	this->setFixedSize(this->size());

#ifndef Q_OS_MAC
	FontHelper::SetFont(this);
#ifdef Q_OS_WIN
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

	//WndManager::DoShape(this);
}

void Scr_Connect::Init()
{
	Setting::Instance()->LoadServer();
	Setting::Instance()->LoadProt();

	// TODO: -1  get actual data
	ui->L_Until->setText("active until\n-");
	ui->L_Amount->setText("-");
	SetOldIp(AuthManager::Instance()->OldIp());
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
	if (srv < 0)		// none
	{
		SetNoSrv();
	}
	else
	{
		const AServer & se = AuthManager::Instance()->GetSrv(srv);
		ui->L_Country->setText(se.name);
		ui->b_Flag->show();	ui->b_FlagBox->show();

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
		ui->L_Percent->show();  ui->L_LOAD->show();
		SetFlag(srv);
	}
}

void Scr_Connect::DwnlStrs()
{

}

void Scr_Connect::UpdNewIp(const QString & s)
{
	ui->L_NewIp->setText(s);
	ui->L_NewIp->show();
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

Scr_Connect::~Scr_Connect()
{
	{
		if (this->isVisible())
		{
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

void Scr_Connect::StartTimer()
{
	if (NULL != _timer_state.get())
		_timer_state->stop();
	_timer_state.reset(new QTimer(this));
	connect(_timer_state.get(), SIGNAL(timeout()), this, SLOT(Timer_CheckState()));
	_timer_state->start(1200);
}

static const char * gs_ConnGreen = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-green.png);\n}";
static const char * gs_ConnRed = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-red.png);\n}";
static const char * gs_ConnYellow = "QLabel\n{\n	border:0px;\n	color: #ffffff;\n	border-image: url(:/imgs/connect-status-yellow.png);\n}";
void Scr_Connect::StatusConnecting()
{
	ui->L_ConnectStatus->setStyleSheet(gs_ConnYellow);
	SetEnabledButtons(false);
}

void Scr_Connect::StatusConnecting(const QString & word)
{
	ModifyWndTitle(word);
	this->StatusConnecting();
}

void Scr_Connect::ModifyWndTitle(const QString & word)
{
	QString s = "Safejumper";
	if (!word.isEmpty())
		s += " " + word;
	this->setWindowTitle(s);
}

void Scr_Connect::Timer_CheckState()
{
//	static int gs_count = 0;
//	++gs_count;
	Ctr_Openvpn::Instance()->CheckState();
//	if (gs_count > 5)
//		_timer_state->stop();
}

void Scr_Connect::SetEnabledButtons(bool enabled)
{
	if (enabled)
	{
		ui->b_Connect->show();
		ui->b_Cancel->hide();
	}
	else
	{
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
	if (NULL != _timer_state.get())
	{
		_timer_state->stop();
		delete _timer_state.release();
	}
	ModifyWndTitle("");
}

std::auto_ptr<Scr_Connect> Scr_Connect::_inst;
Scr_Connect * Scr_Connect::Instance()
{
	if (!_inst.get())
	{
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
	Ctr_Openvpn::Instance()->Start();		// handle visuals inside

}

void Scr_Connect::Clicked_Cancel()
{
	Ctr_Openvpn::Instance()->Stop();
}

void Scr_Connect::Clicked_Jump()
{
	AuthManager::Instance()->Jump();
}

void Scr_Connect::Clicked_Min()
{
	WndManager::Instance()->HideThis(this);
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

void Scr_Connect::LogfileChanged(const QString & pfn)
{
	Ctr_Openvpn::Instance()->LogfileChanged(pfn);
}

void Scr_Connect::ConnectStateChanged(QProcess::ProcessState newState)
{
	log::logt("Scr_Connect::ConnectStateChanged(): newState = " + QString::number(newState));
	Ctr_Openvpn::Instance()->StateChanged(newState);
}

void Scr_Connect::ConnectFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	log::logt("Scr_Connect::ConnectFinished(): exitCode = " + QString::number(exitCode) + " exitStatus = " +  QString::number(exitStatus));
	Ctr_Openvpn::Instance()->Finished(exitCode, exitStatus);
}

void Scr_Connect::ConnectStderr()
{
	Ctr_Openvpn::Instance()->ReadStderr();
}

void Scr_Connect::ConnectStdout()
{
	Ctr_Openvpn::Instance()->ReadStdout();
}

void Scr_Connect::Pressed_Head()
{
	_moving = true;
	_prevMouse = QCursor::pos();
	log::logt("Pressed_Head()");
}

void Scr_Connect::Released_Head()
{
	_moving = false;
	log::logt("Released_Head()");
}

void Scr_Connect::mouseMoveEvent(QMouseEvent* event)
{
	//log::logt("mouseMoveEvent()");
	//if (event->type() == QEvent::MouseMove)
	{
		if (_moving)
		{
			int x = event->x();
			int y = event->y();
			QPoint curr = QCursor::pos();
			if (curr != _prevMouse)
			{
				QPoint d1 = curr - _prevMouse;
				log::logt("moving " + QString::number(d1.x()) + "," + QString::number(d1.y()) );
				this->move(this->pos() + (curr - _prevMouse) );
				_prevMouse = curr;
			}
		}
	}
}

void Scr_Connect::keyPressEvent(QKeyEvent * e)
{
	if(e->key() != Qt::Key_Escape)
		QDialog::keyPressEvent(e);
}








