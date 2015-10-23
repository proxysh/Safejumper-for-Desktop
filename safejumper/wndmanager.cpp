#include "wndmanager.h"

#include <cassert>
#include <QBitmap>
#include <QPainter>

#include "scr_settings.h"
#include "sjmainwindow.h"
#include "scr_map.h"
#include "scr_logs.h"
#include "scr_connect.h"
#include "authmanager.h"
#include "retina.h"
#include "dlg_error.h"
#include "log.h"

WndManager::WndManager()
{

}

WndManager::~WndManager()
{

}

void WndManager::ToPrimary()
{
	if (AuthManager::Instance()->IsLoggedin())
		ToConnect();
	else
		ToLogin();
}

QWidget * WndManager::Primary()
{
	if (AuthManager::Instance()->IsLoggedin())
		return Scr_Connect::Instance();
	else
		return SjMainWindow::Instance();
}

void WndManager::ToLogin()
{
	QWidget * from = ScrVisible();
	if (SjMainWindow::Instance() != from)
	{
		trans(from, SjMainWindow::Instance());
	}
	else
	{
		ToFront(SjMainWindow::Instance());		// activate it
	}
}

void WndManager::ToConnect()
{
	QWidget * from = ScrVisible();
	if (Scr_Connect::Instance() != from)
	{
		trans(from, Scr_Connect::Instance());
		Scr_Connect::Instance()->SetAccName(AuthManager::Instance()->VpnName());
	}
	else
	{
		ToFront(Scr_Connect::Instance());		// activate it
	}
}

void WndManager::ToSettings()
{
	trans(ScrVisible(), Scr_Settings::Instance());
}

void WndManager::ToLogs()
{
	trans(ScrVisible(), Scr_Logs::Instance());
}

void WndManager::ToMap()
{
	if (AuthManager::Instance()->IsLoggedin())
		trans(ScrVisible(), Scr_Map::Instance());
	else
		ToLogin();
}

std::auto_ptr<WndManager> WndManager::_inst;
WndManager * WndManager::Instance()
{
	if (!_inst.get())
		_inst.reset(new WndManager());
	return _inst.get();
}

void WndManager::DoTrans(QWidget * to)
{
	ApplyCoords(to);
	ToFront(to);
}

void WndManager::ToFront(QWidget * w)
{
	if (w)
	{
		w->show();
		w->raise();
		w->activateWindow();
	}
}

void WndManager::ToFront()
{
	ToFront(ScrVisible());
}

void WndManager::trans(QWidget * from, QWidget * to)
{
	if (from)
	{
		if (from != to)
			SaveAndHide(from);
	}
	else
		CloseAll();
	DoTrans(to);
}

void WndManager::trans(const QPoint & newpos, QWidget * to)
{
	_x = newpos.x();
	_y = newpos.y();
	DoTrans(to);
}

QPoint WndManager::CurrPos()
{
	return QPoint(_x, _y);
}

void WndManager::CloseAll()
{
	int visible = 0;
	if (Scr_Connect::IsExists())
	if (Scr_Connect::Instance()->isVisible())
	{
		SaveAndHide(Scr_Connect::Instance());
		++visible;
	}
	if (SjMainWindow::IsExists())
	if (SjMainWindow::Instance()->isVisible())
	{
		SaveAndHide(SjMainWindow::Instance());
		++visible;
	}
	if (Scr_Logs::IsExists())
	if (Scr_Logs::Instance()->isVisible())
	{
		SaveAndHide(Scr_Logs::Instance());
		++visible;
	}
	if (Scr_Map::IsExists())
	if (Scr_Map::Instance()->isVisible())
	{
		SaveAndHide(Scr_Map::Instance());
		++visible;
	}
	if (Scr_Settings::IsExists())
	if (Scr_Settings::Instance()->isVisible())
	{
		SaveAndHide(Scr_Settings::Instance());
		++visible;
	}
	//assert(visible < 2);
}

void WndManager::SaveAndHide(QWidget * from)
{
	SaveCoords(from);
	from->hide();
}

void WndManager::HideThis(QWidget * scr)
{
	SaveAndHide(scr);
}

void WndManager::SaveCoords(QWidget * from)
{
	if (!from) return;
	if (!from->isVisible())
		return;
	int tw = from->width();
	//assert(tw > 0);
	int px = from->x() + (from->width() / 2);
	_x = px - 187;  // 187 int = width of primary / 2
	_y = from->y();
}

void WndManager::ApplyCoords(QWidget * to)
{
	int tw = to->width();
	//assert(tw > 0);
	int nx = _x + 187 - (to->width() / 2);

//log::logt(QString().sprintf("ApplyCoords(): moving to (%d,%d)", nx, _y));
	to->move(nx, _y);
}

void WndManager::DoShape(QWidget * d)
{
	d->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	if (IsRetina())
	{
		QSize sz = d->size();
static		QBitmap bmp(d->size());

		QPainter painter(&bmp);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setPen(QColor(Qt::black));
		painter.setBrush(QColor(Qt::black));
		static float R = 3.0f;
//	QRect rec = d->geometry();
		painter.drawRoundedRect(d->geometry(), R, R, Qt::AbsoluteSize);
		d->setMask(bmp);
		log::logt("Cast main form for retina()");
	}
	else
	{
		QString fn = ":/imgs/shape-dlg";
		int h = d->height();
		switch (h)
		{
			case 168:
			case 146: //fn += "-146"; break;
			{
//				static QBitmap b146(":/imgs/shape-dlg-146.bmp");
				static QBitmap b146(":/imgs/shape-dlg-168.bmp");
				d->setMask(b146);
				return;
			}
			case 197:
			case 175: //fn += "-175"; break;
			{
//				static QBitmap b175(":/imgs/shape-dlg-175.bmp");
				static QBitmap b175(":/imgs/shape-dlg-197.bmp");
				d->setMask(b175);
				return;
			}
			case 293:
			case 271: //fn += "-271"; break;
			{
//				static QBitmap b271(":/imgs/shape-dlg-271.bmp");
				static QBitmap b271(":/imgs/shape-dlg-293.bmp");
				d->setMask(b271);
				return;
			}
			case 300:
			case 278: //fn += "-278"; break;
			{
//				static QBitmap b278(":/imgs/shape-dlg-278.bmp");
				static QBitmap b278(":/imgs/shape-dlg-300.bmp");
				d->setMask(b278);
				return;
			}
			case 360:
			case 338:
			default:
			{
				//static QBitmap b338(":/imgs/shape-dlg.bmp");
				static QBitmap b338(":/imgs/shape-dlg-360.bmp");
				d->setMask(b338);
				return;
			}
		}
//		if (IsRetina())
//			fn += "@2x";
		fn += ".bmp";
		d->setMask(QBitmap(fn));
	}

	//QString n = (IsRetina() ? ":/imgs/region-dlg-err@2x.png" : ":/imgs/region-dlg-err.png");
	//QPixmap msk(n);
	//d->setMask(msk.mask());
}

void WndManager::SavePos()
{
	SETTINGS_OBJ;
	settings.setValue("pos", this->CurrPos());
}

QWidget * WndManager::ScrVisible()
{
	QWidget * w = NULL;
	int visible = 0;
	if (Scr_Connect::IsExists())
	if (Scr_Connect::Instance()->isVisible())
	{
		w = Scr_Connect::Instance();
		++visible;
	}
	if (SjMainWindow::IsExists())
	if (SjMainWindow::Instance()->isVisible())
	{
		w = SjMainWindow::Instance();
		++visible;
	}
	if (Scr_Logs::IsExists())
	if (Scr_Logs::Instance()->isVisible())
	{
		w = Scr_Logs::Instance();
		++visible;
	}
	if (Scr_Map::IsExists())
	if (Scr_Map::Instance()->isVisible())
	{
		w = Scr_Map::Instance();
		++visible;
	}
	if (Scr_Settings::IsExists())
	if (Scr_Settings::Instance()->isVisible())
	{
		w = Scr_Settings::Instance();
		++visible;
	}
	//assert(visible < 2);
	return w;
}

void WndManager::HandleConnecting()
{
	// disable buttons
	// change status label to connecting
	Scr_Connect::Instance()->StatusConnecting();
	if (Scr_Map::IsExists())
		Scr_Map::Instance()->StatusConnecting();
	SjMainWindow::Instance()->StatusConnecting();

	// if any form visible
	QWidget * w = ScrVisible();
	if (NULL != w)  // switch to Scr_Connect
		ToPrimary();

	// disable menu buttons

	// change tray icon
	SjMainWindow::Instance()->StatusConnecting();
}

void WndManager::HandleConnected()
{
	ClosePortDlg();
	Scr_Connect::Instance()->StatusConnected();
	SjMainWindow::Instance()->StatusConnected();
}

void WndManager::HandleDisconnected()
{
	Scr_Connect::Instance()->StatusDisconnected();
	if (Scr_Map::IsExists())
		Scr_Map::Instance()->StatusDisconnected();
	SjMainWindow::Instance()->StatusDisconnected();
	// TODO: -0
}

void WndManager::HandleState(const QString & word)
{
	Scr_Connect::Instance()->StatusConnecting(word);
}

void WndManager::ErrMsg(const QString & msg)
{
	this->ToPrimary();
	Dlg_Error dlg(msg, "Error", this->ScrVisible());
	dlg.exec();
}

int WndManager::Confirmation(const QString & msg)
{
	Dlg_confirmation dlg(msg, Primary());
	return dlg.exec();
}

void WndManager::ShowPortDlg()
{
	ToPrimary();
	_DlgPort.reset(new Dlg_confirmation("Connection failed? Let's try another port.", Primary()));
	//_DlgPort->setWindowModality(Qt::ApplicationModal);
	Scr_Connect * w = Scr_Connect::Instance();
	w->connect(_DlgPort.get(), SIGNAL(finished(int)), w, SLOT(PortDlgAction(int)));
	_DlgPort->open();
}

void WndManager::ClosePortDlg()
{
	if (_DlgPort.get() != NULL)
	{
		Scr_Connect * w = Scr_Connect::Instance();
		w->disconnect(_DlgPort.get(), SIGNAL(finished(int)), w, SLOT(PortDlgAction(int)));
		_DlgPort->close();
		delete _DlgPort.release();
	}
}
