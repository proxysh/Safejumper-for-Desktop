#include "scr_map.h"

#include <QListView>
#include <QItemDelegate>
#include <QStandardItemModel>

#include "setting.h"
#include "ui_scr_map.h"
#include "scr_connect.h"
#include "scr_settings.h"
#include "wndmanager.h"
#include "authmanager.h"
#include "scr_connect.h"
#include "ctr_openvpn.h"
#include "log.h"
#include "flag.h"
#include "lvrowdelegate.h"
#include "lvrowdelegateprotocol.h"
#include "fonthelper.h"
#include "sjmainwindow.h"

Scr_Map::Scr_Map(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Scr_Map)
    , _moving(false)
{
	ui->setupUi(this);
	this->setFixedSize(this->size());
#ifndef Q_OS_MAC
	FontHelper::SetFont(this);
#endif

#ifdef Q_OS_WIN
	ui->lv_Location->setMinimumWidth(300);
	ui->lv_Location->setMinimumHeight(400);
#endif
	_default = ui->L_Mark->pos();
/*
	ui->b_Tmp->move(0,0);
	ui->b_Tmp->resize(ui->L_Map->width(), ui->L_Map->height());
*/
	ui->e_x->hide(); ui->e_y->hide();
	ui->b_Tmp->hide();

	RePopulateLocations();

	for (size_t k = 0; k < Setting::GetAllProt().size(); ++k)
		ui->dd_Protocol->addItem(Setting::GetAllProt().at(k));
	ui->dd_Protocol->setItemText(0, PROTOCOL_SELECTION_STR);	// HACK: -2

//	QPoint p0 = _WndStart = pos();
//	WndManager::DoShape(this);
//	QPoint p1 = pos();
//		move(p0);
	qApp->installEventFilter(this);


	ui->dd_Location->setView(ui->lv_Location);
	ui->dd_Protocol->setView(ui->lv_Protocol);
	ui->dd_Protocol->setItemDelegate(new LvRowDelegateProtocol(this));
	ui->dd_Location->setItemDelegate(new LvRowDelegate(this));

//	setMouseTracking(true); // E.g. set in your constructor of your widget.

}


static const QString gs_Individual = "QListView\n{\nbackground: solid #f7f7f7;\n}\nQListView::item\n{\n/*color: #202020;*/\ncolor: #f7f7f7;	/*invisible*/\nheight: 24;\nborder-image: url(:/imgs/dd-row.png);\nbackground: solid #f7f7f7;\n}\nQListView::item:selected\n{\nborder-image: url(:/imgs/dd-selectionrow-322.png);\n}\n\n";
static const QString gs_Hubs = "QListView\n{\nbackground: solid #f7f7f7;\n}\nQListView::item\n{\n/*color: #202020;*/\ncolor: #f7f7f7;	/*invisible*/\nheight: 24;\nborder-image: url(:/imgs/dd-row.png);\nbackground: solid #f7f7f7;\n}\nQListView::item:selected\n{\nborder-image: url(:/imgs/dd-selectionrow-244.png);\n}\n\n";
void Scr_Map::SetRowStyle(bool show_nodes)
{
	if (show_nodes)
		ui->lv_Location->setStyleSheet(gs_Individual);
	else
		ui->lv_Location->setStyleSheet(gs_Hubs);
}

void Scr_Map::Changed_xy()
{
/*
	int x = ui->e_x->text().toInt();
	int y = ui->e_y->text().toInt();
	ui->L_Mark->move(x, y);
*/
}

void Scr_Map::RePopulateLocations()
{
	// store previously chosen id
	int oldN = ui->dd_Location->count();
	int ixoldsrv = -1;
	QString oldsrv;
	if (oldN > 1)
	{
		ixoldsrv = CurrSrv();
		if (ixoldsrv > -1)
		{
			oldsrv = AuthManager::Instance()->GetSrv(ixoldsrv).name;	//ui->dd_Location->currentText();
		}
	// and clear the list
		ui->dd_Location->setCurrentIndex(0);
		for (int k = oldN; k > 0; --k)
			ui->dd_Location->removeItem(k);
	}

	// populate with the actual servers
	_IsShowNodes = Setting::Instance()->IsShowNodes();
	SetRowStyle(_IsShowNodes);

	const std::vector<AServer> & srvs = (
		_IsShowNodes ?
		AuthManager::Instance()->GetAllServers() :
		AuthManager::Instance()->GetHubs() );
	for (size_t k = 0, sz = srvs.size(); k < sz; ++k)
	{   // add individual srv / hub node item
		// TODO: -1 format the line
		int srv = _IsShowNodes ? k : AuthManager::Instance()->ServerIdFromHubId(k);
		int ping = AuthManager::Instance()->PingFromSrvIx(srv);
		QString s0 = srvs[k].name;
		QString s1;
		int load = (int)srvs[k].load.toDouble();

		s1 += QString::number(load) + "%";
		if (ping > -1)
			s1 += " / " + QString::number(ping) + "ms";
		// New Zealand Hub
//		if (srvs[k].name.length() < 12)
//			s0 += "\t";
//		s0 += "\t\t" + s1 ;	// + "\t "
//		s0 += "\t" + s1;
//		s0 += "\t\t\t\t";
		s0 += "\t\t";				// HACK: override width
		ui->dd_Location->addItem(s0);
//		ui->dd_Location->addItem("");
	}

	// try to reselect the chosen server
	int toselect = 0;
	if (oldN > 1)
	{
		if (ixoldsrv > -1)
		{
			QString newname;
			if (_IsShowNodes)
			{   // hubs -->> all
				newname = AuthManager::Instance()->GetAllServers().at(ixoldsrv).name;
				if (newname == oldsrv)
					toselect = ixoldsrv + 1;	// hub item in the same location
				else
				{
					int hubnewix = AuthManager::Instance()->SrvIxFromName(oldsrv);
					if (hubnewix > -1)
						toselect = hubnewix + 1;
					else
						// does not exist anymore
						;   // TODO: -2 closest
				}
			}
			else
			{   // all -->> hubs
				int newhubix = AuthManager::Instance()->HubIxFromSrvName(oldsrv);
				if (newhubix > -1)
					toselect = newhubix + 1;
			}
		}
	}
	ui->dd_Location->setCurrentIndex(toselect);
}

void Scr_Map::closeEvent(QCloseEvent * event)
{
	event->ignore();
	WndManager::Instance()->HideThis(this);
}

Scr_Map::~Scr_Map()
{
	delete ui;
}

std::auto_ptr<Scr_Map> Scr_Map::_inst;
Scr_Map * Scr_Map::Instance()
{
	if (!_inst.get())
		_inst.reset(new Scr_Map());
	return _inst.get();
}

int Scr_Map::CurrSrv()
{
	int srv = -1;
	int ix = ui->dd_Location->currentIndex();
	if(ix > 0)
	{
		if (_IsShowNodes)
			srv = ix - 1;   // TODO: -0 inadequate during repopulation during changes of number of servers
		else
			srv = AuthManager::Instance()->ServerIdFromHubId(ix - 1);
	}
	return srv;
}

void Scr_Map::ToScr_Connect()
{
	int srv = CurrSrv();
	Scr_Connect::Instance()->SetServer(srv);
	Scr_Connect::Instance()->SetProtocol(ui->dd_Protocol->currentIndex() - 1);
	WndManager::Instance()->ToPrimary();
}

void Scr_Map::ToScr_Settings()
{
	WndManager::Instance()->ToSettings();
}

void Scr_Map::Clicked_b_Connect()
{
	Ctr_Openvpn::Instance()->Start();
}

void Scr_Map::Clicked_b_Tmp()
{
	QString s;
	 int x = ui->e_x->text().toInt();
	 int y = ui->e_y->text().toInt();
	 //s.sprintf("%u, %u", _curr.x(), _curr.y());
	 s.sprintf("%u, %u", x, y);
	 qDebug() << s;
}



// Implement in your widget
void Scr_Map::mouseMoveEvent(QMouseEvent *event){
   _curr = event->pos();
	QString s;
   s.sprintf("%u, %u", _curr.x(), _curr.y());
	this->setWindowTitle(s);
}

static const char * const gs_stIcon1 = "QLabel\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/imgs/l-1.png);\n}";
static const char * const gs_stIcon2 = "QLabel\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/imgs/l-2.png);\n}";
static const char * const gs_stIcon2inact = "QLabel\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/imgs/l-2-inactive.png);\n}";
static const char * const gs_stIconV = "QLabel\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/imgs/l-v.png);\n}";
void Scr_Map::Changed_dd_Protocol(int ix)
{
	if (ix > 0)
	{
		ui->dd_Location->setEnabled(true);
		ui->L_1->setStyleSheet(gs_stIconV);
		if (ui->dd_Location->currentIndex() > 0)
			ui->L_2->setStyleSheet(gs_stIconV);
		else
			ui->L_2->setStyleSheet(gs_stIcon2);
	}
	else
	{
		ui->dd_Location->setEnabled(false);
		ui->L_1->setStyleSheet(gs_stIcon1);
		ui->L_2->setStyleSheet(gs_stIcon2inact);
	}
	if (Scr_Connect::IsExists())
		Scr_Connect::Instance()->SetProtocol(ix - 1);
	Setting::Instance()->SaveProt(ix - 1);
}

void Scr_Map::Changed_dd_Sever(int ix)
{
	int ixsrv = -1;
	if (ix > 0)
	{
		ui->L_2->setStyleSheet(gs_stIconV);
		if (Scr_Connect::IsExists())
		{
			if (_IsShowNodes)
				ixsrv = ix - 1;
			else
				ixsrv = AuthManager::Instance()->ServerIdFromHubId(ix - 1);
		}
		AuthManager::Instance()->SetNewIp("");
	}
	else
	{
		ui->L_2->setStyleSheet(gs_stIcon2);
	}
	AServer se = AuthManager::Instance()->GetSrv(ixsrv);
	QString newsrv = se.name;
	//= ui->dd_Location->currentText();
	if (Scr_Connect::IsExists())
		Scr_Connect::Instance()->SetServer(ixsrv);
	Setting::Instance()->SaveServer(ixsrv, newsrv);

	DisplayMark(se.name);


}

void Scr_Map::DisplayMark(const QString & name)
{
	QPoint p = flag::CoordsFromSrvName(name);
	if (p.x() == 0 && p.y() == 0)
		p = _default;
	ui->L_Mark->move(p);
	ui->L_Mark->setText(flag::ShortName(name));
}

void Scr_Map::SetServer(int ixsrv)
{
	int toselect = 0;
	if (ixsrv > -1)
	{
		if (_IsShowNodes)
			toselect = ixsrv + 1;
		else
			toselect = AuthManager::Instance()->HubIdFromItsSrvId(ixsrv) + 1;
	}
	ui->dd_Location->setCurrentIndex(toselect);
}

void Scr_Map::SetProtocol(int ix)
{
	ui->dd_Protocol->setCurrentIndex(ix + 1);
	//qobject_cast<QStandardItemModel *>(ui->dd_Protocol->model())->item(ix + 1)->setCheckState(Qt::Checked);
	//qobject_cast<QStandardItemModel *>(ui->dd_Protocol->model())->item(ix + 1)->set
}

int Scr_Map::CurrProto()
{
	return ui->dd_Protocol->currentIndex() - 1;
}

void Scr_Map::StatusConnecting()
{
	ui->b_Connect->setEnabled(false);
	// TODO: -1 CANCEL button
}

void Scr_Map::StatusConnected()
{
	;
}

void Scr_Map::StatusDisconnected()
{
	ui->b_Connect->setEnabled(true);
}

void Scr_Map::keyPressEvent(QKeyEvent * e)
{
	if(e->key() != Qt::Key_Escape)
		QDialog::keyPressEvent(e);
}


bool Scr_Map::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type())
	{
		case QEvent::MouseMove:
		{
			if (_moving)
			{
				QPoint d = QCursor::pos() - _CursorStart;
				if (d.x() != 0 || d.y() != 0)
				{
					QPoint NewAbs = _WndStart + d;
					this->move(NewAbs);
				}
			}
			return false;
		}
		case QEvent::MouseButtonRelease:
		{
			_moving = false;
//			_WndStart = pos();
			return false;
		}
		default:
			return QDialog::eventFilter(obj, event);
	}
}

void Scr_Map::Pressed_Head()
{
	_WndStart = this->pos();
	_CursorStart = QCursor::pos();
	_moving = true;
}

void Scr_Map::Clicked_Min()
{
	WndManager::Instance()->HideThis(this);
}

void Scr_Map::Clicked_Cross()
{
	SjMainWindow::Instance()->DoClose();
}


