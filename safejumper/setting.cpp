#include "setting.h"

#include <stdexcept>

#include "scr_settings.h"
#include "scr_map.h"
#include "scr_connect.h"
#include "scr_map.h"
#include "common.h"
#include "authmanager.h"
#include "sjmainwindow.h"



//In future, we’ll add things such as “OpenVPN with XOR TCP 448” or “OpenVPN with TOR UDP 4044”.


std::vector<QString> Setting::_protocols[ENCRYPTION_COUNT];
std::vector<int> Setting::_ports[ENCRYPTION_COUNT];

Setting::Setting()
{
	_default_dns[0] = "146.185.134.104";
	_default_dns[1] = "192.241.172.159";
}

Setting::~Setting()
{}

void Setting::SetDefaultDns(const QString & dns1, const QString & dns2)
{
	_default_dns[0] = dns1;
	_default_dns[1] = dns2;
}

void Setting::PopulateColls(std::vector<QString> & v_strs, std::vector<int> & v_ports, size_t sz, const char ** protocols, const int * ports)
{
	if (v_strs.empty())
	{
		for (size_t k = 0; k < sz; ++k)
		{
			v_strs.push_back(protocols[k]);
			v_ports.push_back(ports[k]);
		}
	}
}

const std::vector<QString> & Setting::GetAllProt()
{
	int enc = Encryption();
	if (_protocols[enc].empty())
	{
		switch (enc)
		{
			case ENCRYPTION_RSA:
			{
				static const char * gs_protocols [] =
				{
					"OpenVPN TCP 80"
					, "OpenVPN TCP 110"
					, "OpenVPN TCP 443"

					, "OpenVPN TCP 843"

					, "OpenVPN UDP 53"

					, "OpenVPN UDP 1194"
					, "OpenVPN UDP 1443"
					, "OpenVPN UDP 8080"
					, "OpenVPN UDP 9201"
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

				size_t sz = sizeof(gs_protocols)/sizeof(gs_protocols[0]);
				PopulateColls(_protocols[enc], _ports[enc], sz, gs_protocols, gs_ports);
				break;
			}

			case ENCRYPTION_OBFS_TOR:
			{
				static const char * gs_protocols1 [] =
				{
					"Obfsproxy TCP 888"
				};
				static const int gs_ports1 [] =
				{
					888
				};
				size_t sz = sizeof(gs_protocols1)/sizeof(gs_protocols1[0]);
				PopulateColls(_protocols[enc], _ports[enc], sz, gs_protocols1, gs_ports1);
				break;
			}
			case ENCRYPTION_ECC:
			{
				static const char * gs_protocols2 [] =
				{
					"OpenVPN TCP 465 ECC"
					, "OpVPN TCP 44144 ECC"
				};
				static const int gs_ports2 [] =
				{
					465
					, 44144
				};
				size_t sz = sizeof(gs_protocols2)/sizeof(gs_protocols2[0]);
				PopulateColls(_protocols[enc], _ports[enc], sz, gs_protocols2, gs_ports2);
				break;
			}
			case ENCRYPTION_ECCXOR:
			{
				static const char * gs_protocols2 [] =
				{
					"VPN TCP 995 ECC+XOR"
				};
				static const int gs_ports2 [] =
				{
					995
				};
				size_t sz = sizeof(gs_protocols2)/sizeof(gs_protocols2[0]);
				PopulateColls(_protocols[enc], _ports[enc], sz, gs_protocols2, gs_ports2);
				break;
			}
			default:
				throw std::runtime_error("invalid encryption index");
		}
	}
	return _protocols[enc];
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

bool Setting::IsBlockOnDisconnect()
{
	return Scr_Settings::Instance()->Is_cb_BlockOnDisconnect();
}

bool Setting::IsFixDns()
{
	return Scr_Settings::Instance()->Is_cb_FixDnsLeak();
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

int Setting::Encryption()
{
	int encryption = Scr_Settings::Instance()->Encryption();
	if (encryption < 0 || encryption >= ENCRYPTION_COUNT)
		throw std::runtime_error("invalid encryption index");
	return encryption;
}

const char * Setting::EncText(size_t enc)
{
	static const char * g_ar [] = {
		"RSA 4096-bit"
		, "TOR's obfsproxy"
		, "ECC"
		, "ECC + XOR"
	};
	if (enc >= ENCRYPTION_COUNT || enc >= (sizeof(g_ar)/sizeof(g_ar[0])))
		enc = ENCRYPTION_RSA;
	return g_ar[enc];
}

QString Setting::EncryptionIx()
{
	int enc = Encryption();
	QString s;
	if (enc > 0)
		s = QString::number(enc);
	return s;
}

QString Setting::ProtocolSettingsName()
{
	return "dd_Protocol_ix" + EncryptionIx();
}

QString Setting::ProtocolSettingsStrName()
{
	return "dd_Protocol_str" + EncryptionIx();
}

QString Setting::LocationSettingsName()
{
	return "dd_Location_ix" + EncryptionIx();
}

QString Setting::LocationSettingsStrName()
{
	return "dd_Location_str" + EncryptionIx();
}

static bool _loading_protocol = false;
void Setting::SaveProt(int ix)
{
	if (_loading_protocol)
		return;
	SETTINGS_OBJ;
	settings.setValue(ProtocolSettingsName(), ix);
	QString s;
	if (ix > -1 && ix < (int)GetAllProt().size())
		s = GetAllProt().at(ix);
	settings.setValue(ProtocolSettingsStrName(), s);
}

int Setting::LoadProt()
{
	_loading_protocol = true;
	SETTINGS_OBJ;
	int ix = settings.value(ProtocolSettingsName(), -1).toInt();
	if (ix > -1)
	{
		if (ix >= (int)GetAllProt().size())
			ix = -1;
		else
		{
			QString s = settings.value(ProtocolSettingsStrName(), "").toString();
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

	_loading_protocol = false;
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

const QString & Setting::CurrProtoStr()
{
	return ProtoStr(CurrProto());
}

int Setting::CurrProto()
{
	// TODO: -2 from saved settings when Scr_Map unavailable
	return Scr_Map::Instance()->CurrProto();
}

void Setting::SaveServer(int ixsrv, const QString & newsrv)
{
	SETTINGS_OBJ;
	settings.setValue(LocationSettingsName(), ixsrv);
	settings.setValue(LocationSettingsStrName(), newsrv);
}

void Setting::LoadServer()
{
	if (AuthManager::Instance()->GetAllServers().empty())
		return;		// cannot select in empty list

	SETTINGS_OBJ;
	int savedsrv = settings.value(LocationSettingsName(), -1).toInt();
	QString savedname = settings.value(LocationSettingsStrName(), "undefined").toString();

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
	int encryption = Scr_Settings::Instance()->Encryption();
	if (encryption < 0 || encryption > ENCRYPTION_COUNT)
		throw std::runtime_error("invalid encryption index");
	std::vector<int> & v_ports = _ports[encryption];
	if (ix > -1 && ix < (int)v_ports.size())
		p = v_ports[ix];
	return QString::number(p);
}

void Setting::SwitchToNextPort()
{
	int ix = Scr_Map::Instance()->CurrProto();
	++ix;
	int encryption = Scr_Settings::Instance()->Encryption();
	if (encryption < 0 || encryption > ENCRYPTION_COUNT)
		throw std::runtime_error("invalid encryption index");
	std::vector<int> & v_ports = _ports[encryption];
	if (ix >= (int)v_ports.size())
		ix = 0;
	Scr_Map::Instance()->SetProtocol(ix);
}

void Setting::SwitchToNextNode()
{
	Scr_Map::Instance()->SwitchToNextNode();
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
	if (description.contains("udp", Qt::CaseInsensitive))
		return "udp";
	else
		return "tcp";
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



