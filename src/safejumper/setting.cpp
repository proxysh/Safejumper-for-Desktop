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

#include "setting.h"

#include <stdexcept>

#include "scr_settings.h"
#include "scr_map.h"
#include "connectiondialog.h"
#include "scr_map.h"
#include "common.h"
#include "authmanager.h"
#include "loginwindow.h"
#include "trayiconmanager.h"


//In future, we’ll add things such as “OpenVPN with XOR TCP 448” or “OpenVPN with TOR UDP 4044”.


std::vector<QString> Setting::mProtocols[ENCRYPTION_COUNT];
std::vector<int> Setting::mPorts[ENCRYPTION_COUNT];

Setting::Setting()
    :mTesting(false)
{
    mDefaultDNS[0] = "146.185.134.104";
    mDefaultDNS[1] = "192.241.172.159";
}

Setting::~Setting()
{}

void Setting::setDefaultDNS(const QString & dns1, const QString & dns2)
{
    mDefaultDNS[0] = dns1;
    mDefaultDNS[1] = dns2;
}

QString Setting::defaultDNS1()
{
    return mDefaultDNS[0];
}

QString Setting::defaultDNS2()
{
    return mDefaultDNS[1];
}

void Setting::PopulateColls(std::vector<QString> & v_strs, std::vector<int> & v_ports, size_t sz, const char ** protocols, const int * ports)
{
    if (v_strs.empty()) {
        for (size_t k = 0; k < sz; ++k) {
            v_strs.push_back(protocols[k]);
            v_ports.push_back(ports[k]);
        }
    }
}

const std::vector<QString> & Setting::allProtocols()
{
    int enc = encryption();
    if (mProtocols[enc].empty()) {
        switch (enc) {
        case ENCRYPTION_RSA: {
            static const char * gs_protocols [] = {
                "TCP 80 (RSA)"
                , "TCP 110 (RSA)"
                , "TCP 443 (RSA)"

                , "TCP 843 (RSA)"

                , "UDP 53 (RSA)"

                , "UDP 1194 (RSA)"
                , "UDP 1443 (RSA)"
                , "UDP 8080 (RSA)"
                , "UDP 9201 (RSA)"
            };
            static const int gs_ports [] = {
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
            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols, gs_ports);
            break;
        }

        case ENCRYPTION_TOR_OBFS2: {
            static const char * gs_protocols1 [] = {
                "TCP 888 (RSA+TOR)"
            };
            static const int gs_ports1 [] = {
                888
            };
            size_t sz = sizeof(gs_protocols1)/sizeof(gs_protocols1[0]);
            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols1, gs_ports1);
            break;
        }
        case ENCRYPTION_TOR_OBFS3: {
            static const char *gs_protocols1 [] = {
                "TCP 898 (RSA+TOR)"
            };
            static const int gs_ports1 [] = {
                898
            };
            size_t sz = sizeof(gs_protocols1)/sizeof(gs_protocols1[0]);
            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols1, gs_ports1);
            break;
        }
        case ENCRYPTION_TOR_SCRAMBLESUIT: {
            static const char *gs_protocols1 [] = {
                "TCP 988 (RSA+TOR)"
            };
            static const int gs_ports1 [] = {
                988
            };
            size_t sz = sizeof(gs_protocols1)/sizeof(gs_protocols1[0]);
            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols1, gs_ports1);
            break;
        }
        case ENCRYPTION_ECC: {
            static const char * gs_protocols2 [] = {
                "TCP 465 (ECC)"
            };
            static const int gs_ports2 [] = {
                465
            };
            size_t sz = sizeof(gs_protocols2)/sizeof(gs_protocols2[0]);
            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols2, gs_ports2);
            break;
        }
        case ENCRYPTION_ECCXOR: {
            static const char * gs_protocols2 [] = {
                "TCP 995 (ECC+XOR)"
            };
            static const int gs_ports2 [] = {
                995
            };
            size_t sz = sizeof(gs_protocols2)/sizeof(gs_protocols2[0]);
            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols2, gs_ports2);
            break;
        }
        default:
            throw std::runtime_error("invalid encryption index");
        }
    }
    return mProtocols[enc];
}

const std::vector<int> & Setting::allPorts()
{
    const std::vector<QString> & pr = allProtocols();		// force init
    if (!pr.empty()) {
        int enc = encryption();
        return mPorts[enc];
    } else {
        return mPorts[0];
    }
}

std::auto_ptr<Setting> Setting::mInstance;
Setting * Setting::instance()
{
    if (!mInstance.get())
        mInstance.reset(new Setting());
    return mInstance.get();
}

void Setting::cleanup()
{
    if (mInstance.get() != NULL)
        delete mInstance.release();
}

bool Setting::exists()
{
    return (mInstance.get() != NULL);
}

bool Setting::showNodes()
{
    return Scr_Settings::Instance()->Is_cb_ShowNodes();
}

bool Setting::disableIPv6()
{
    return Scr_Settings::Instance()->Is_cb_DisableIpv6();
}

bool Setting::autoconnect()
{
    return Scr_Settings::Instance()->Is_cb_AutoConnect();
}

bool Setting::detectInsecureWifi()
{
    return Scr_Settings::Instance()->Is_cb_InsecureWiFi();
}

bool Setting::blockOnDisconnect()
{
    return Scr_Settings::Instance()->Is_cb_BlockOnDisconnect();
}

bool Setting::fixDns()
{
    return Scr_Settings::Instance()->Is_cb_FixDnsLeak();
}

bool Setting::testing()
{
    return mTesting;
}

void Setting::setTesting(bool value)
{
    mTesting = value;
}

bool Setting::startup()
{
    return Scr_Settings::Instance()->Is_cb_Startup();
}

bool Setting::reconnect()
{
    return Scr_Settings::Instance()->Is_cb_Reconnect();
}

void Setting::setShowNodes(bool v)
{
    SaveCb("cb_ShowNodes", v);

    if (Scr_Map::IsExists()) {
        int old = Scr_Map::Instance()->CurrSrv();
        Scr_Map::Instance()->RePopulateLocations();
    }
    TrayIconManager::instance()->constructConnectToMenu();
}

int Setting::encryption()
{
    int encryption = Scr_Settings::Instance()->Encryption();
    if (encryption < 0 || encryption >= ENCRYPTION_COUNT)
        throw std::runtime_error("invalid encryption index");
    return encryption;
}

const char * Setting::encryptionName(size_t enc)
{
    static const char * g_ar [] = {
        "RSA 4096-bit",
        "RSA + TOR (obfs2)",
        "RSA + TOR (obfs3)",
        "RSA + TOR (scramblesuit)"
        "ECC (secp384r1)",
        "ECC + XOR",
    };
    if (enc >= ENCRYPTION_COUNT || enc >= (sizeof(g_ar)/sizeof(g_ar[0])))
        enc = ENCRYPTION_RSA;
    return g_ar[enc];
}

QString Setting::EncryptionIx()
{
    int enc = encryption();
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
void Setting::setProtocol(int ix)
{
    if (_loading_protocol)
        return;
    QSettings settings;
    settings.setValue(ProtocolSettingsName(), ix);
    QString s;
    if (ix > -1 && ix < (int)allProtocols().size())
        s = allProtocols().at(ix);
    settings.setValue(ProtocolSettingsStrName(), s);
}

void Setting::loadProtocol()
{
    _loading_protocol = true;
    QSettings settings;
    int ix = settings.value(ProtocolSettingsName(), -1).toInt();
    if (ix > -1) {
        if (ix >= (int)allProtocols().size())
            ix = -1;
        else {
            QString s = settings.value(ProtocolSettingsStrName(), "").toString();
            if (s != allProtocols().at(ix))
                ix = -1;
        }
    }
    if (ix < 0) {
        ix = rand() % allProtocols().size();
    }
    Scr_Map::Instance()->SetProtocol(ix);   // will trigger if differs
    if (ix < 0)		 // forse update - handle case when not differs
        ConnectionDialog::instance()->setProtocol(ix);

    _loading_protocol = false;
}

static QString gs_Empty = "";
const QString & Setting::protocolName(int ix)
{
    if (ix > -1)
        return allProtocols().at(ix);
    else
        return gs_Empty;
}

const QString & Setting::currentProtocolName()
{
    return protocolName(currentProtocol());
}

int Setting::currentProtocol()
{
    // TODO: -2 from saved settings when Scr_Map unavailable
    return Scr_Map::Instance()->CurrProto();
}

void Setting::setServer(int ixsrv, const QString & newsrv)
{
    QSettings settings;
    settings.setValue(LocationSettingsName(), ixsrv);
    settings.setValue(LocationSettingsStrName(), newsrv);
}

void Setting::loadServer()
{
    if (AuthManager::instance()->currentEncryptionServers().empty())
        return;		// cannot select in empty list

    QSettings settings;
    int savedsrv = settings.value(LocationSettingsName(), -1).toInt();
    QString savedname = settings.value(LocationSettingsStrName(), "undefined").toString();

    int ixsrv = -1;
    // verify that the sever exists
    if (savedsrv > -1) {
        AServer  sr1 = AuthManager::instance()->getServer(savedsrv);
        if (!sr1.name.isEmpty()) {
            if (sr1.name == savedname)
                ixsrv = savedsrv;
            else if (savedname != "undefined")
                ixsrv = AuthManager::instance()->serverIxFromName(savedname);
        } else {
            if (savedname != "undefined")
                ixsrv = AuthManager::instance()->serverIxFromName(savedname);
        }
    }

    if (ixsrv < 0)
        ixsrv = AuthManager::instance()->getServerToJump();

    Scr_Map * sm = Scr_Map::Instance(); // initiate population of the Location drop-down; will call Setting::IsShowNodes() which will initiate scr_settings and load checkboxes

    sm->SetServer(ixsrv);   // will trigger if differs
    if (ixsrv < 0)					// force update - handle case when not differs
        ConnectionDialog::instance()->setServer(ixsrv);
}

QString Setting::serverAddress()
{
    QString s;
    int ix = Scr_Map::Instance()->CurrSrv();
    if (ix > -1)
        s = AuthManager::instance()->getServer(ix).address;
    return s;
}

int Setting::serverID()
{
    return Scr_Map::Instance()->CurrSrv();
}

QString Setting::port()
{
    int ix = Scr_Map::Instance()->CurrProto();
    int p = 80;
    int encryption = Scr_Settings::Instance()->Encryption();
    if (encryption < 0 || encryption > ENCRYPTION_COUNT)
        throw std::runtime_error("invalid encryption index");
    std::vector<int> & v_ports = mPorts[encryption];
    if (ix > -1 && ix < (int)v_ports.size())
        p = v_ports[ix];
    return QString::number(p);
}

#ifdef MONITOR_TOOL
void Setting::InitLoop()
{
    _ixStartPort = Scr_Map::Instance()->CurrProto();
    if (_ixStartPort < 0) {
        _ixStartPort = 0;
        Scr_Map::Instance()->SetProtocol(_ixStartPort);
    }

    _idStartNode = Scr_Map::Instance()->CurrSrv();		// -1 if not selected
    if (_idStartNode < 0) {
        const std::vector<size_t> & srvs = AuthManager::Instance()->GetAllServers();
        if (!srvs.empty()) {
            _idStartNode = srvs.front();
            Scr_Map::Instance()->SetServer(_idStartNode);
        }
    }
}

bool Setting::SwitchToNext()
{
    bool NotAllProcessed = true;
    int ix = DetermineNextPort();
    if (ix != _ixStartPort) {
        Scr_Map::Instance()->SetProtocol(ix);
    } else {
        _ixStartPort = 0;
        Scr_Map::Instance()->SetProtocol(_ixStartPort);
        Scr_Map::Instance()->SwitchToNextNode();

        int idSrv = Scr_Map::Instance()->CurrSrv();		// -1 if not selected
        if (_idStartNode == idSrv) {
            // TODO: -1 change encryption
            NotAllProcessed = false;
        }
    }
    return NotAllProcessed;
}
#endif	// MONITOR_TOOL

int Setting::DetermineNextPort()
{
    int ix = Scr_Map::Instance()->CurrProto();
    ++ix;
    int encryption = Scr_Settings::Instance()->Encryption();
    if (encryption < 0 || encryption > ENCRYPTION_COUNT)
        throw std::runtime_error("invalid encryption index");
    std::vector<int> & v_ports = mPorts[encryption];
    if (ix >= (int)v_ports.size())
        ix = 0;
    return ix;
}

void Setting::switchToNextPort()
{
    int ix = DetermineNextPort();
    Scr_Map::Instance()->SetProtocol(ix);
}

void Setting::switchToNextNode()
{
    Scr_Map::Instance()->SwitchToNextNode();
}

QString Setting::localPort()
{
    QString p = Scr_Settings::Instance()->LocalPort();
    if (p.isEmpty())
        p = "6842";
    return p;
}

QString Setting::tcpOrUdp()
{
    QString description = protocolName(Scr_Map::Instance()->CurrProto());
    if (description.contains("udp", Qt::CaseInsensitive))
        return "udp";
    else
        return "tcp";
}

QString Setting::dns1()
{
    return Scr_Settings::Instance()->Dns1();
}

QString Setting::dns2()
{
    return Scr_Settings::Instance()->Dns2();
}

UVec Setting::forwardPorts()
{
    USet s = Scr_Settings::Instance()->Ports();
    UVec v(s.begin(), s.end());
    std::sort(v.begin(), v.end());
    return v;
}

static const char * gs_upd_name = "LastUpdMsg";
static const char * gs_undefined = "undefined";
bool Setting::checkForUpdates()
{
    bool is = true;
    QSettings settings;
    QString saved = settings.value(gs_upd_name, gs_undefined).toString();
    if (saved != gs_undefined) {
        uint curr = QDateTime::currentDateTimeUtc().toTime_t();
        bool ok;
        uint old = saved.toUInt(&ok);
        if (ok) {
            int d = curr - old;
            static const int day = 24 * 60 * 60;
            if (d < day)
                is = false;
        }
    }
    return is;
}

void Setting::updateMessageShown()
{
    QSettings settings;
    uint t = QDateTime::currentDateTimeUtc().toTime_t();
    settings.setValue(gs_upd_name, QString::number(t));
}



