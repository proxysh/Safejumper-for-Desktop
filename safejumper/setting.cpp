#include "setting.h"

#include "scr_settings.h"
#include "scr_map.h"
#include "scr_connect.h"
#include "scr_map.h"
#include "common.h"
#include "authmanager.h"
#include "sjmainwindow.h"

static const char * gs_protocols [] = {
	"OpenVPN TCP 80"
	, "OpenVPN TCP 110"
	, "OpenVPN TCP 443"

	, "OpenVPN TCP 843"

	, "OpenVPN UDP 53"

	, "OpenVPN UDP 1194"
	, "OpenVPN UDP 1443"
	, "OpenVPN UDP 8080"
	, "OpenVPN UDP 9201"
	//In future, we’ll add things such as “OpenVPN with XOR TCP 448” or “OpenVPN with TOR UDP 4044”.
};

static const int gs_ports [] =
{
	80
	, 110
	, 443
	, 843
	, 53
	, 1194
	, 1443
	, 8080
	, 9201
};

std::vector<QString> Setting::_protocols;

Setting::Setting()
{
	_default_dns[0] = "146.185.134.104";
	_default_dns[1] = "192.241.172.159";
}

Setting::~Setting()
{

}

void Setting::SetDefaultDns(const QString & dns1, const QString & dns2)
{
	_default_dns[0] = dns1;
	_default_dns[1] = dns2;
}

const std::vector<QString> & Setting::GetAllProt()
{
	if (_protocols.empty())
	{
		for (size_t k = 0, sz = sizeof(gs_protocols)/sizeof(gs_protocols[0]); k < sz; ++k)   //
			_protocols.push_back(gs_protocols[k]);
	}
	return _protocols;
}

std::auto_ptr<Setting> Setting::_inst;
Setting * Setting::Instance()
{
	if (!_inst.get())
		_inst.reset(new Setting());
	return _inst.get();
}

bool Setting::IsShowNodes()
{
	return Scr_Settings::Instance()->Is_cb_ShowNodes();
}

bool Setting::IsDisableIPv6()
{
	return Scr_Settings::Instance()->Is_cb_DisableIpv6();
}

bool Setting::IsAutoconnect()
{
	return Scr_Settings::Instance()->Is_cb_AutoConnect();
}

bool Setting::IsInsecureWifi()
{
	return Scr_Settings::Instance()->Is_cb_InsecureWiFi();
}

bool Setting::IsStartup()
{
	return Scr_Settings::Instance()->Is_cb_Startup();
}

bool Setting::IsReconnect()
{
	return Scr_Settings::Instance()->Is_cb_Reconnect();
}

void Setting::ToggleShowNodes(bool v)
{
	SaveCb("cb_ShowNodes", v);

	if (Scr_Map::IsExists())
	{
		int old = Scr_Map::Instance()->CurrSrv();
		Scr_Map::Instance()->RePopulateLocations();
	}
	SjMainWindow::Instance()->ConstructConnecttoMenu();
}

void Setting::SaveProt(int ix)
{
	SETTINGS_OBJ;
	settings.setValue("dd_Protocol_ix", ix);
	QString s;
	if (ix > -1 && ix < (int)GetAllProt().size())
		s = GetAllProt().at(ix);
	settings.setValue("dd_Protocol_str", s);
}

int Setting::LoadProt()
{
	SETTINGS_OBJ;
	int ix = settings.value("dd_Protocol_ix", -1).toInt();
	if (ix > -1)
	{
		if (ix >= (int)GetAllProt().size())
			ix = -1;
		else
		{
			QString s = settings.value("dd_Protocol_str", "").toString();
			if (s != GetAllProt().at(ix))
				ix = -1;
		}
	}
	if (ix < 0)
	{
		ix = rand() % GetAllProt().size();
	}
	Scr_Map::Instance()->SetProtocol(ix);   // will trigger if differs
	if (ix < 0)		 // forse update - handle case when not differs
		Scr_Connect::Instance()->SetProtocol(ix);
	return ix;
}

static QString gs_Empty = "";
const QString & Setting::ProtoStr(int ix)
{
	if (ix > -1)
		return GetAllProt().at(ix);
	else
		return gs_Empty;
}

void Setting::SaveServer(int ixsrv, const QString & newsrv)
{
	SETTINGS_OBJ;
	settings.setValue("dd_Location_ix", ixsrv);
	settings.setValue("dd_Location_str", newsrv);
}

void Setting::LoadServer()
{
	SETTINGS_OBJ;
	int savedsrv = settings.value("dd_Location_ix", -1).toInt();
	QString savedname = settings.value("dd_Location_str", "undefined").toString();

	int ixsrv = -1;
	// verify that the sever exists
	if (savedsrv > -1)
	{
		AServer  sr1 = AuthManager::Instance()->GetSrv(savedsrv);
		if (!sr1.name.isEmpty())
		{
			if (sr1.name == savedname)
				ixsrv = savedsrv;
			else
				if (savedname != "undefined")
					ixsrv = AuthManager::Instance()->SrvIxFromName(savedname);
		}
		else
		{
			if (savedname != "undefined")
				ixsrv = AuthManager::Instance()->SrvIxFromName(savedname);
		}
	}

	if (ixsrv < 0)
		ixsrv = AuthManager::Instance()->SrvToJump();

	Scr_Map * sm = Scr_Map::Instance(); // initiate population of the Location drop-down; will call Setting::IsShowNodes() which will initiate scr_settings and load checkboxes
	sm->SetServer(ixsrv);   // will trigger if differs
	if (ixsrv < 0)					// force update - handle case when not differs
		Scr_Connect::Instance()->SetServer(ixsrv);
}

QString Setting::Server()
{
	QString s;
	int ix = Scr_Map::Instance()->CurrSrv();
	if (ix > -1)
		s = AuthManager::Instance()->GetSrv(ix).address;
	return s;
}

QString Setting::Port()
{
	int ix = Scr_Map::Instance()->CurrProto();
	int p = 80;
	if (ix > -1 && ix < (int)(sizeof(gs_ports)/sizeof(gs_ports[0])))
		p = gs_ports[ix];
	return QString::number(p);
}

QString Setting::LocalPort()
{
	QString p = Scr_Settings::Instance()->LocalPort();
	if (p.isEmpty())
		p = "6842";
	return p;
}

QString Setting::Protocol()
{
	QString description = ProtoStr(Scr_Map::Instance()->CurrProto());
	if (description.contains("TCP", Qt::CaseInsensitive))
		return "tcp";
	else
		return "udp";
	// TODO: -2 other protocol types
}

QString Setting::Dns1()
{
	return Scr_Settings::Instance()->Dns1();
}

QString Setting::Dns2()
{
	return Scr_Settings::Instance()->Dns2();
}

UVec Setting::ForwardPorts()
{
	USet s = Scr_Settings::Instance()->Ports();
	UVec v(s.begin(), s.end());
	std::sort(v.begin(), v.end());
	return v;
}

static const char * gs_upd_name = "LastUpdMsg";
static const char * gs_undefined = "undefined";
bool Setting::IsCheckForUpdates()
{
	bool is = true;
	SETTINGS_OBJ;
	QString saved = settings.value(gs_upd_name, gs_undefined).toString();
	if (saved != gs_undefined)
	{
		uint curr = QDateTime::currentDateTimeUtc().toTime_t();
		bool ok;
		uint old = saved.toUInt(&ok);
		if (ok)
		{
			int d = curr - old;
			static const int day = 24 * 60 * 60;
			if (d < day)
				is = false;
		}
	}
	return is;
}

void Setting::UpdateMsgShown()
{
	SETTINGS_OBJ;
	uint t = QDateTime::currentDateTimeUtc().toTime_t();
	settings.setValue(gs_upd_name, QString::number(t));
}



