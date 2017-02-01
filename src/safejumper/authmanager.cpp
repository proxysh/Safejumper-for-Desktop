#include "authmanager.h"

#include <cassert>
#include <algorithm>
#include <set>
#include <stdexcept>

#include <QDomDocument>

#include "pingwaiter.h"
#include "setting.h"
#include "openvpnmanager.h"
#include "scr_map.h"
#include "loginwindow.h"
#include "osspecific.h"
#include "log.h"
#include "flag.h"
#include "common.h"
#include "update.h"
#include "version.h"
#include "wndmanager.h"

std::auto_ptr<AuthManager> AuthManager::_inst;
AuthManager * AuthManager::Instance()
{
    if (!_inst.get())
        _inst.reset(new AuthManager());
    return _inst.get();
}

bool AuthManager::exists()
{
    return (_inst.get() != NULL);
}

void AuthManager::cleanup()
{
    if (_inst.get() != NULL)
        delete _inst.release();
}

AuthManager::AuthManager()
    :
    _logged(false)
    , _seeded(false)
    , _CancelLogin(false)
    , _ip_attempt_count(0)
{
    connect(OpenvpnManager::instance(), SIGNAL(gotNewIp(QString)),
            this, SLOT(setNewIp(QString)));
}

AuthManager::~AuthManager()
{
    for (size_t k = 0; k < _workers.size(); ++k) {
        if (_timers.at(k) != NULL) {
            _timers.at(k)->stop();
            delete _timers.at(k);
        }
        /*              if (_workers.at(k) != NULL && _waiters.at(k) != NULL)
                        {
                                SjMainWindow * m = SjMainWindow::Instance();
                        }*/
        if (_workers.at(k) != NULL) {
            if (_workers.at(k)->state() != QProcess::NotRunning)
                _workers.at(k)->terminate();
            delete _workers.at(k);
        }
        if (_waiters.at(k) != NULL)
            delete _waiters.at(k);
    }

    if (_th_oldip.get() != NULL) {
        if (_th_oldip->isRunning())
            _th_oldip->terminate();
        delete _th_oldip.get();
    }

    if (_forwarder.get() != NULL) {
        // TODO: -1 terminate connections; disconnect signals
        delete _forwarder.release();
    }

    // TODO: -0 terminate Network Manager
//      _nam
}

bool AuthManager::loggedIn()
{
    // TODO: -0 not implemented
    return _logged;
}

void AuthManager::login(const QString & name, const QString & password)
{
    _aclogin = name;
    _acnpsw = password;

    _vpnlogin.clear();
    _vpnpsw.clear();                    // TODO: -2 secure clear
    _logged = false;
    _CancelLogin = false;
    log::logt("Starting login with name '" + QUrl::toPercentEncoding(name, "", "") + "'");

    _reply.reset(_nam.get(BuildRequest(QUrl("https://proxy.sh/access.php?u=" + QUrl::toPercentEncoding(name, "", "") + "&p=" + QUrl::toPercentEncoding(password, "", "")))));
    connect(_reply.get(), SIGNAL(finished()), this, SLOT(loginFinished()));
}

void AuthManager::cancel()
{
    _CancelLogin = true;
    if(NULL != _reply.get()) {
        _reply->abort();
    }
}

void AuthManager::logout()
{
    cancel();
    _logged = false;
    _vpnlogin.clear();
    _vpnpsw.clear();
    _aclogin.clear();
    _acnpsw.clear();            // TODO: -2 secure clear
}

AServer AuthManager::GetSrv(int id)
{
    AServer s;
    //assert(id > -1);
    if (id > -1) {
        if (id < _servers.size())
            s = _servers.at(id);
    }
    return s;
}

AServer AuthManager::GetHub(int idhub)
{
    int idsrv = -1;
    if (idhub > -1 && idhub < _hubs.size())
        idsrv = ServerIdFromHubId(idhub);
    return GetSrv(idsrv);
}

void AuthManager::setNewIp(const QString & ip)
{
    static const QString self = "127.0.0.1";
    if (ip != self) {
        _newip = ip;
        emit newIpLoaded(ip);
    }
}

const std::vector<size_t> & AuthManager::currentEncryptionServers()
{
    int enc = Setting::Encryption();
    return _srv_ids[enc];
}

const std::vector<size_t> & AuthManager::currentEncryptionHubs()
{
    if (!_servers.empty() && _hubs.empty()) {
        for (size_t k = 0; k < _servers.size(); ++k) {
            if (_servers[k].name.contains("Hub", Qt::CaseInsensitive)) {
                _hubs.push_back(_servers[k]);
                _HubToServer.push_back(k);                      // the same as _hub_ids[0]
                _hub_ids[0].push_back(k);
                _SrvidToHubId.insert(IIMap::value_type(k, _hubs.size() - 1));
                std::string cleared = flag::ClearName(_servers[k].name).toStdString();          // QString cleared = flag::ClearName(_servers[k].name);
                _HubClearedId.insert(std::make_pair(cleared, _hubs.size() - 1));
            }
        }
    }
    int enc = Setting::Encryption();
    return _hub_ids[enc];
}

int AuthManager::HubidForServerNode(size_t srv)
{
    int hub = -1;
    AServer & rs = _servers.at(srv);
    std::string cleared = flag::ClearName(rs.name).toStdString();       // QString cleared = flag::ClearName(rs.name);
    std::map<std::string, size_t>::iterator it = _HubClearedId.find(cleared);
    if (it != _HubClearedId.end())
        hub = (int)(*it).second;
    return hub;
}

const std::vector<std::pair<bool, int> > & AuthManager::GetLvl0()
{
    PrepareLevels();
    return _level0;
}

void AuthManager::PrepareLevels()
{
    // TODO: -1 special hub for boost
    if (!_servers.empty() && _level0.empty()) {
        const std::vector<size_t> & h = currentEncryptionHubs();
        std::set<int> hub_srvids;
        for (size_t k =0; k < h.size(); ++k) {
            int srv = h.at(k);  //ServerIdFromHubId(k);
            hub_srvids.insert(srv);
            std::vector<int> v;
            v.push_back(srv);
            _level1.insert(std::make_pair(k, v));
        }

        const std::vector<size_t> & sr = currentEncryptionServers();
        for (size_t k = 0; k < sr.size(); ++k) {
            int srv = sr.at(k);
            std::set<int>::iterator it = hub_srvids.find(srv);
            if (it != hub_srvids.end()) {
                _level0.push_back(std::make_pair(true, HubIdFromItsSrvId(*it)));        // aready at lvl 1
            } else {    // just server: lvl0 or lvl1
                int chub = HubidForServerNode(k);
                if (chub > -1) {
                    std::map<int, std::vector<int> >::iterator it2 = _level1.find(chub);
                    if (it2 != _level1.end()) {
                        std::vector<int> & rv = (*it2).second;
                        rv.push_back(k);
                    }
                } else {
                    _level0.push_back(std::make_pair(false, k));
                }
            }
        }
    }
}

const std::vector<int> & AuthManager::GetLvl1(size_t hub)
{
    PrepareLevels();
    std::map<int, std::vector<int> >::iterator it = _level1.find(hub);
    if (it == _level1.end())
        return _fake;
    else
        return (*it).second;
}

size_t AuthManager::ServerIdFromHubId(size_t ixHub)
{
    return _HubToServer.at(ixHub);
}

int AuthManager::HubIxFromSrvName(const QString & srv)
{
    int ix = -1;
    if (!srv.isEmpty()) {
        //int ixsrv = SrvIxFromName(srv);        // hint
        int ixSpace = -1;
        for (int k = srv.length() - 1; k > 0; --k) {
            if (srv[k] == ' ') {
                ixSpace = k;
                break;
            }
        }
        QString base = (ixSpace > -1 ? srv.left(ixSpace) : srv);
        QString s3 = base + " Hub";
        int ix2 = SrvIxFromName(s3);    // try 'Canada' + ' Hub'
        if (ix2 > -1)   // convert id into hub id
            ix = HubIdFromItsSrvId(ix2);
    }
    return ix;
}

int AuthManager::HubIdFromItsSrvId(int ixsrv)
{
    int hub = -1;
    if (ixsrv > -1) {
        IIMap::const_iterator ci = _SrvidToHubId.find(ixsrv);
        if (ci != _SrvidToHubId.end())
            hub = (*ci).second;
    }
    return hub;
}

int AuthManager::SrvIxFromName(const QString & srv)
{
    int ix = -1;
    if (_SrvNameToId.empty() && !_servers.empty()) {
        for (size_t k = 0, sz = _servers.size(); k < sz; ++k)
            _SrvNameToId.insert(SIMap::value_type(_servers.at(k).name.toStdString(), k));
    }
    SIMap::iterator it = _SrvNameToId.find(srv.toStdString());
    if (it != _SrvNameToId.end())
        ix = (*it).second;
    return ix;
}

int AuthManager::PingFromSrvIx(int srv)
{
    int pn = -1;
    if (srv > -1 && srv < _pings.size())
        pn =_pings.at(srv);
    return pn;
}

void AuthManager::ClearColls()
{
    _hubs.clear();
    _level0.clear();
    _level1.clear();
    _HubToServer.clear();
    _SrvidToHubId.clear();
    _SrvNameToId.clear();
    _servers.clear();
    _pings.clear();
}

void AuthManager::clearReply()
{
    if (_reply.get() != NULL) {
        _reply->abort();
        _reply->deleteLater();
        _reply.release();
    }
}

void AuthManager::getEccServerNames()
{
    clearReply();
    // https://api.proxy.sh/safejumper/get_ecc/name
    _reply.reset(AuthManager::Instance()->_nam.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/get_ecc/name"))));
    connect(_reply.get(), SIGNAL(finished()),
            this, SLOT(processEccServerNamesXml()));
}

void AuthManager::processEccServerNamesXml()
{
    QString message;
    bool err = processServerNamesForEncryptionType(ENCRYPTION_ECC, message);

    if (err)
        log::logt("Error getting ecc names: " + message);

    // do not get obfs addresses: proceed
    getAccountType();

    if (!err) {
        // clone ECC nodes into ECC+XOR
        // TODO: -2 is there a specific for ECC+XOR  API page?
        _srv_ids[ENCRYPTION_ECCXOR].clear();
        _srv_ids[ENCRYPTION_ECCXOR].assign(_srv_ids[ENCRYPTION_ECC].begin(), _srv_ids[ENCRYPTION_ECC].end());
        ForceRepopulation(ENCRYPTION_ECC);
        ForceRepopulation(ENCRYPTION_ECCXOR);
        if (Setting::Encryption() != ENCRYPTION_RSA)
            Setting::Instance()->LoadServer();
    }
}

void AuthManager::getObfsServerNames()
{
    clearReply();
    // https://api.proxy.sh/safejumper/get_obfs/name
    _reply.reset(AuthManager::Instance()->_nam.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/get_obfs/name"))));
    connect(_reply.get(), SIGNAL(finished()),
            this, SLOT(processObfsServerNamesXml()));
}

void AuthManager::processObfsServerNamesXml()
{
    QString message;
    bool err = processServerNamesForEncryptionType(ENCRYPTION_OBFS_TOR, message);

    if (err)
        log::logt("Error getting obfs names: " + message);
    // do not get obfs addresses: proceed
    getEccServerNames();

    if (!err)
        ForceRepopulation(ENCRYPTION_OBFS_TOR);

    return;


#if 0
    bool err = false;
    out_msg.clear();
    if (_reply->error() != QNetworkReply::NoError) {
        log::logt(_reply->errorString());
    } else {
        QByteArray ba = _reply->readAll();

//{QFile f("/tmp/obfsname.xml");
//f.open(QIODevice::WriteOnly);
//f.write(ba);
//f.flush();
//f.close();}

        /*
        <?xml version="1.0"?>
        <root>
                <0>U.S. California 3</0>
                <1>U.S. Georgia 1</1>

                <130>Boost - Singapore - SoftLayer</130>
        </root>
        */
        QDomDocument doc;
        std::vector<QString> v;
        QString w(ba);
        if (!doc.setContent(w, &out_msg)) {
            int p0 = w.indexOf("<root>");
            int p1 = w.indexOf('<', p0 + 1);
            int end = w.indexOf("</root>", p1 + 1);
            if (end > -1) {
                for (int t = p1; t > -1 && t < end; ) {
                    int p2 = w.indexOf('>', t + 1);
                    int p3 = w.indexOf("</", p2 + 1);
                    QString internal = w.mid(p2 +1, p3 - p2 - 1);
                    if (!internal.isEmpty())
                        v.push_back(internal);
                    t = w.indexOf('<', p3 + 1);
                }
            } else {
                err = true;
                out_msg = "Error parsing Obfs name XML\n" + out_msg;
            }
        } else {
            QDomNodeList roots = doc.elementsByTagName("root");
            if (roots.size() > 0) {
                QDomNode root = roots.item(0);
                QDomNodeList chs = root.childNodes();
                if (!chs.isEmpty()) {
                    for (int k = 0, sz = chs.size(); k < sz; ++k) {
                        QDomNode n = chs.at(k);
                        QString s = n.toElement().text();
                        v.push_back(s);
                    }
                } else {
                    err = true;
                    out_msg = "empty obfs name list";
                }
            } else {
                err = true;
                out_msg = "Missing root node";
            }
        }

        if (!err && !v.empty()) {
            _obfs_names.swap(v);
            MatchObfsServers();
        }
    }

    // do not get obfs addresses: proceed
    //  StartDwnl_AccType();
    StartDwnl_EccName();

    // force update of locations: if needed, previously empy
    if (Setting::IsExists() && Scr_Map::IsExists()) {
        Scr_Map::Instance()->RePopulateLocations();
        // TODO: -0 LoadServer()
    }
#endif
}

//void AuthManager::StartDwnl_EccxorName()
//{
//      ClearReply();
//      // https://api.proxy.sh/safejumper/get_ecc/name
//      _reply.reset(AuthManager::Instance()->_nam.get(BuildRequest(
//              QUrl("https://api.proxy.sh/safejumper/get_ecc/name"))));
//      SjMainWindow * sj = SjMainWindow::Instance();
//      sj->connect(_reply.get(), SIGNAL(finished()), sj, SLOT(Finished_EccxorName()));
//}

//void AuthManager::StartDwnl_ObfsAddr()
//{
//      ClearReply();
//      // https://api.proxy.sh/safejumper/get_obfs/name
//      _reply.reset(AuthManager::Instance()->_nam.get(BuildRequest(
//              QUrl("https://api.proxy.sh/safejumper/get_obfs/name"))));
//      SjMainWindow * sj = SjMainWindow::Instance();
//      sj->connect(_reply.get(), SIGNAL(finished()), sj, SLOT(AccTypeFinishedZZ()));
//}

void AuthManager::getAccountType()
{
    clearReply();
    // https://api.proxy.sh/safejumper/account_type/VPNusername/VPNpassword
    _reply.reset(AuthManager::Instance()->_nam.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/account_type/"
                          + QUrl::toPercentEncoding(AuthManager::Instance()->VpnName(), "", "")
                          + "/" + QUrl::toPercentEncoding(AuthManager::Instance()->VpnPsw(), "", "")))));
    connect(_reply.get(), SIGNAL(finished()),
            this, SLOT(processAccountTypeXml()));
}

void AuthManager::getExpirationDate()
{
    clearReply();
    // https://api.proxy.sh/safejumper/expire_date/VPNusername/VPNpassword
    _reply.reset(AuthManager::Instance()->_nam.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/expire_date/"
                          + QUrl::toPercentEncoding(AuthManager::Instance()->VpnName(), "", "")
                          + "/" + QUrl::toPercentEncoding(AuthManager::Instance()->VpnPsw(), "", "")))));
    connect(_reply.get(), SIGNAL(finished()),
            this, SLOT(processExpirationXml()));
}

void AuthManager::checkUpdates()
{
    QString us(SJ_UPDATE_URL);
    if (!us.isEmpty()) {
        _reply_update.reset(AuthManager::Instance()->_nam.get(BuildRequest(QUrl(us))));
        connect(_reply_update.get(), SIGNAL(finished()),
                this, SLOT(processUpdatesXml()));
    }
}

void AuthManager::getOldIP()
{
    ++_ip_attempt_count;
    log::logt("StartDwnl_OldIp() attempt " + QString::number(_ip_attempt_count));
    static const QString us = "https://proxy.sh/ip.php";
    _reply_IP.reset(AuthManager::Instance()->_nam.get(BuildRequest(QUrl(us))));
    connect(_reply_IP.get(), SIGNAL(finished()),
            this, SLOT(processOldIP()));
}

void AuthManager::getDns()
{
    clearReply();
    // https://api.proxy.sh/safejumper/get_dns
    //<?xml version="1.0"?>
    //<root>
    //  <dns>146.185.134.104</dns>
    //  <dns>192.241.172.159</dns>
    //</root>
    _reply.reset(AuthManager::Instance()->_nam.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/get_dns"))));
    connect(_reply.get(), SIGNAL(finished()),
            this, SLOT(processDnsXml()));
}

void AuthManager::processAccountTypeXml()
{
    QString message;
    if (_reply->error() != QNetworkReply::NoError) {
        log::logt(_reply->errorString());
        return;
    }
    QByteArray ba = _reply->readAll();
    if (ba.isEmpty()) {
        log::logt("Cannot get account info. Server response is empty.");
        return;
    }
    // parse XML response

    // <?xml version="1.0"?>
    //<root><package>10</package><email>aaa@gmail.com</email></root>
    QDomDocument doc;
    if (!doc.setContent(QString(ba), &message)) {
        log::logt("Error parsing XML account info\n" + message);
        return;
    }
    QDomNodeList nlpackage = doc.elementsByTagName("package");
    if (nlpackage.size() <= 0) {
        log::logt("Missing package amount");
        return;
    }
    QDomNode n = nlpackage.item(0);
    QString amount = "$" + n.toElement().text();

    QDomNodeList nl = doc.elementsByTagName("email");
    if (nl.size() <= 0) {
        log::logt("Missing email in account XML");
        return;
    }
    n = nl.item(0);
    QString email = n.toElement().text();
    log::logt("Got account e-mail " + email + " and amount " + amount);
    emit amountLoaded(amount);
    emit emailLoaded(email);
    /*
    QFile f("/tmp/acc.xml");
    f.open(QIODevice::WriteOnly);
    f.write(ba);
    f.flush();
    f.close();
    */

    getExpirationDate();
}

void AuthManager::processExpirationXml()
{
    QString message;
    if (_reply->error() != QNetworkReply::NoError) {
        log::logt(_reply->errorString());
        return;
    }
    QByteArray ba = _reply->readAll();
    if (ba.isEmpty()) {
        log::logt("Cannot get expiration info. Server response is empty.");
        return;
    }
    // parse XML response
    /*
    {QFile f("/tmp/expire.xml");
    f.open(QIODevice::WriteOnly);
    f.write(ba);
    f.flush();
    f.close();}
    */
    // <?xml version="1.0"?>
    // <root><expire_date>2015-05-28</expire_date></root>
    QDomDocument doc;
    QString until = "--";
    if (!doc.setContent(QString(ba), &message)) {
        log::logt("Error parsing XML expiration info\n" + message);
        return;
    }
    QDomNodeList nl = doc.elementsByTagName("expire_date");
    if (nl.size() <= 0) {
        log::logt("Missing expiration info");
        return;
    }
    QDomNode n = nl.item(0);
    until = n.toElement().text();

    emit untilLoaded(until);

    getDns();
}

void AuthManager::processDnsXml()
{
    if (_reply->error() != QNetworkReply::NoError) {
        log::logt(_reply->errorString());
        return;
    }
    QByteArray ba = _reply->readAll();
    if (ba.isEmpty()) {
        log::logt("Cannot get DNS info. Server response is empty.");
        return;
    }
    // parse XML response

    // <?xml version="1.0"?>
    // <root>
    //  <dns>146.185.134.104</dns>
    //  <dns>192.241.172.159</dns>
    // </root>
    QDomDocument doc;
    QString msg;
    if (!doc.setContent(QString(ba), &msg)) {
        log::logt("Error parsing XML DNS info\n" + msg);
        return;
    }
    QDomNodeList nl = doc.elementsByTagName("dns");
    if (nl.size() <= 0) {
        log::logt("Missing DNS nodes");
        return;
    }
    QString dns[2];
    for (int k = 0; k < 2 && k < nl.size(); ++k) {
        QDomNode n = nl.item(k);
        dns[k] = n.toElement().text();
    }
    if (!dns[0].isEmpty() || !dns[1].isEmpty())
        Setting::Instance()->SetDefaultDns(dns[0], dns[1]);

    clearReply();               // TODO: -2 further processing here
}

void AuthManager::processUpdatesXml()
{
    if (_reply_update->error() != QNetworkReply::NoError) {
        log::logt(_reply_update->errorString());
        return;
    }
    QByteArray ba = _reply_update->readAll();

    /*
    QByteArray ba =
    "<?xml version=\"1.1\" encoding=\"UTF-8\"?>"
    "<version>"
      "<stable>3.0</stable>"
      "<build>24</build>"
      "<files>"
            "<file url=\"/safejumper.exe\"/>"
      "</files>"
      "<date>2015-08-05</date>"
    "</version>";
    */
    if (ba.isEmpty()) {
        log::logt("Cannot get Updates info. Server response is empty.");
        return;
    }
    // parse XML response

    // <?xml version="1.1" encoding="UTF-8"?>
    // <version>
    //  <stable>3.0</stable>
    //  <build>23</build>
    //  <files>
    //    <file url="/safejumper.exe"/>
    //  </files>
    //  <date>2015-08-05</date>
    // </version>
    QDomDocument doc;
    QString msg;
    if (!doc.setContent(QString(ba), &msg)) {
        log::logt("Error parsing XML Updates info\n" + msg);
        return;
    }
    QDomNodeList nl = doc.elementsByTagName("build");
    if (nl.size() <= 0) {
        log::logt("Missing 'build' node");
        return;
    }
    QDomNode n = nl.item(0);
    QString ss = n.toElement().text();
    if (!ss.isEmpty()) {
        bool ok;
        int upd = ss.toInt(&ok);
        if (ok && SJ_BUILD_NUM < upd) {
            int result = WndManager::Instance()->Confirmation("New version " + ss + " available. Update?");
            Setting::Instance()->UpdateMsgShown();
            if (result == QDialog::Accepted)
                launchUpdateUrl();           // TODO: -2 auto update self
        }
    }
}

bool AuthManager::processServerNamesForEncryptionType(int enc, QString & out_msg)
{
    out_msg.clear();
    QStringList names = extractNames(out_msg);

    if (out_msg.isEmpty() && !names.isEmpty())
        populateServerIdsFromNames(names, _srv_ids[enc]);

    return !out_msg.isEmpty();
}

void AuthManager::ForceRepopulation(int enc)
{
    // force update of locations: if needed, previously empy
    if (Setting::Encryption() == enc) {
        if (Setting::IsExists() && Scr_Map::IsExists()) {
            Scr_Map::Instance()->RePopulateLocations();
            // TODO: -0 LoadServer()
        }
    }
}

QStringList AuthManager::extractNames(QString & out_msg)
{
    QStringList names;
    if (_reply->error() != QNetworkReply::NoError) {
        log::logt("Network error:" + _reply->errorString());
        out_msg = "Network error: " + _reply->errorString();
    } else {
        QByteArray ba = _reply->readAll();

//{QFile f("/tmp/obfsname.xml");
//f.open(QIODevice::WriteOnly);
//f.write(ba);
//f.flush();
//f.close();}

        /*
        <?xml version="1.0"?>
        <root>
                <0>U.S. California 3</0>
                <1>U.S. Georgia 1</1>

                <130>Boost - Singapore - SoftLayer</130>
        </root>
        */

        QString w(ba);

        int p0 = w.indexOf("<root>");
        int p1 = w.indexOf('<', p0 + 1);
        int end = w.indexOf("</root>", p1 + 1);
        if (end > -1) {
            for (int t = p1; t > -1 && t < end; ) {
                int p2 = w.indexOf('>', t + 1);
                int p3 = w.indexOf("</", p2 + 1);
                QString internal = w.mid(p2 +1, p3 - p2 - 1);
                if (!internal.isEmpty())
                    names.append(internal);
                t = w.indexOf('<', p3 + 1);
            }
        } else {
            out_msg = "Error parsing Obfs name XML\n" + out_msg;
        }
    }
    return names;
}


void AuthManager::populateServerIdsFromNames(QStringList names, std::vector<size_t> & found)
{
    std::vector<size_t> paid;
    for (size_t j = 0, ses = _servers.size(); j < ses; ++j) {
        if (names.contains(_servers.at(j).name)) {
            paid.push_back(j);
        }
    }
    std::sort(paid.begin(), paid.end());
    found.swap(paid);
}

QString AuthManager::processServersXml()
{
    QString message;
    ClearColls();
    _vpnlogin.clear();
    _vpnpsw.clear();
    if (_reply->error() != QNetworkReply::NoError) {
        _logged = false;
        return _reply->errorString();
    } else {
        QByteArray ba = _reply->readAll();
        if (ba.isEmpty()) {
            _logged = false;
            return "Cannot log in with this name and password pair. Server response is empty.";
        }

        // parse XML response
        QDomDocument doc;
        QString login, psw;
        if (!doc.setContent(QString(ba), &message)) {
            _logged = false;
            return "Error parsing server XML response\n" + message;
        } else {
            QDomNodeList nlLogin = doc.elementsByTagName("username");
            if (nlLogin.size() > 0) {
                QDomNode n = nlLogin.item(0);
                login = n.toElement().text();
            } else {
                _logged = false;
                return "Missing credentials";
            }
        }
        QDomNodeList nl = doc.elementsByTagName("password");
        if (nl.size() > 0) {
            QDomNode n = nl.item(0);
            psw = n.toElement().text();
        } else {
            _logged = false;
            return "Missing credentials password";
        }
        nl = doc.elementsByTagName("server");
        if (nl.size() > 0) {
            for (int k = 0; k < nl.size(); ++k) {
                QDomNode se = nl.item(k);
                QDomElement adr = se.firstChildElement("address");
                QDomElement loc = se.firstChildElement("location");
                QDomElement load = se.firstChildElement("server_load");
                if (adr.isNull() || loc.isNull() || load.isNull())
                    continue;
                AServer s2;
                s2.address = adr.text();
                s2.name = loc.text();
                s2.load = load.text();
                _servers.push_back(s2);
                _srv_ids[0].push_back(_servers.size() - 1);
            }
            _vpnlogin = login;
            _vpnpsw = psw;
            _logged = true;
        } else {
            _logged = false;
            return "Incorrect credentials. Make sure to use your VPN credentials and not your email.";
        }
    }

    // force hubs
    const std::vector<size_t> & hr = AuthManager::currentEncryptionHubs();
    if (hr.empty())
        log::logt("Cannot parse hubs");

    getObfsServerNames();
//  StartDwnl_AccType();

    pingAllServers();

    return message;
}

void AuthManager::pingAllServers()
{
    // Fill _pinks with as many -1 entries as there are servers
    if (_pings.empty())
        _pings.assign(_servers.size(), -1);
    if (_pings.size() < _servers.size()) {
        for (size_t k = 0, n = _servers.size() - _pings.size(); k < n; ++k)
            _pings.push_back(-1);
    }
    for (size_t k = 0; k < _servers.size(); ++k)
        _toping.push(k);

    if (_workers.empty()) {
        _inprogress.assign(PINGWORKERS_NUM, 0);
        LoginWindow * m = LoginWindow::Instance();
        for (size_t k = 0; k < PINGWORKERS_NUM; ++k) {
            _workers.push_back(NULL);
            _waiters.push_back(new PingWaiter(k, m));
            _timers.push_back(new QTimer());
        }
    }

    _pings_loaded = false;
    for (size_t k = 0; k < _workers.size() && !_toping.empty(); ++k)
        StartWorker(k);
}

void AuthManager::StartWorker(size_t id)
{
    if (!_toping.empty()) {
        size_t srv = _toping.front();
        _toping.pop();

        //log::logt(QString("Pop srv ") + QString::number(srv));

        _inprogress.at(id) = srv;
        LoginWindow * m = LoginWindow::Instance();

        if (_workers.at(id) !=NULL) {
            m->disconnect(_workers.at(id), SIGNAL(finished(int,QProcess::ExitStatus)),
                          _waiters.at(id), SLOT(PingFinished(int,QProcess::ExitStatus)));
            m->disconnect(_workers.at(id), SIGNAL(error(QProcess::ProcessError)),
                          _waiters.at(id), SLOT(PingError(QProcess::ProcessError)));
            if (_workers.at(id)->state() != QProcess::NotRunning)
                _workers.at(id)->terminate();
        }
        std::auto_ptr<QProcess> raii(_workers.at(id));          // force delete if any
        _workers.at(id) = new QProcess(m);
        m->connect(_workers.at(id), SIGNAL(finished(int,QProcess::ExitStatus)), _waiters.at(id), SLOT(PingFinished(int,QProcess::ExitStatus)));
        m->connect(_workers.at(id), SIGNAL(error(QProcess::ProcessError)), _waiters.at(id), SLOT(PingError(QProcess::ProcessError)));
        OsSpecific::Instance()->StartPing(*_workers.at(id), _servers.at(srv).address);
        m->connect(_timers.at(id), SIGNAL(timeout()), _waiters.at(id), SLOT(Timer_Terminate()));
        _timers.at(id)->setSingleShot(true);
        _timers.at(id)->start(PINGWORKER_MAX_TIMEOUT);
    } else {
        if (!_pings_loaded) {
            _pings_loaded = true;
            Scr_Map::Instance()->RePopulateLocations();         // load pings
        }
    }
}

void AuthManager::PingComplete(size_t idWaiter)
{
    _timers.at(idWaiter)->stop();
    int p = OsSpecific::Instance()->ExtractPing(*_workers.at(idWaiter));
//      log::logt(_servers.at(_inprogress.at(idWaiter)).address + " Got ping " + QString::number(p));
    _pings.at(_inprogress.at(idWaiter)) = p;
    StartWorker(idWaiter);
}

void AuthManager::PingErr(size_t idWaiter)
{
    _timers.at(idWaiter)->stop();
    int p = OsSpecific::Instance()->ExtractPing(*_workers.at(idWaiter));
//      log::logt(_servers.at(_inprogress.at(idWaiter)).address + " ping process error, extracted ping: " + QString::number(p));
    _pings.at(_inprogress.at(idWaiter)) = p;
    StartWorker(idWaiter);
}

void AuthManager::PingTerminate(size_t idWaiter)
{
    _workers.at(idWaiter)->terminate();
    int p = OsSpecific::Instance()->ExtractPing(*_workers.at(idWaiter));
//      log::logt(_servers.at(_inprogress.at(idWaiter)).address + " ping process terminated, extracted ping: " + QString::number(p));
    _pings.at(_inprogress.at(idWaiter)) = p;
    StartWorker(idWaiter);
}

void AuthManager::DoSeed()
{
    if (!_seeded) {
        srand(time(NULL));
        _seeded = true;
    }
}

std::vector<int> AuthManager::GetPings(const std::vector<size_t> & toping)
{
    std::vector<int> v;
    v.assign(toping.size(), -1);
    if (_pings.empty())
        log::logt("GetPings(): Empty pings collection");
    else {
        for (size_t k = 0; k < toping.size(); ++k) {
            if (toping.at(k) >= _pings.size())
                log::logt("GetPings(): Server id greater than size of pings coll");
            else
                v.at(k) = _pings.at(toping.at(k));
        }
    }
    return v;
}

typedef std::pair<int, size_t> IUPair;
typedef std::vector<IUPair> IUVec;
static bool PCmp(const IUPair & a, const IUPair & b)
{
    return a.first < b.first;
}

int AuthManager::getServerToJump()
{
    int srv = -1;
    if (!_servers.empty()) {
        int prev = Scr_Map::Instance()->CurrSrv();
        std::vector<size_t> toping;     // ix inside _servers
        if (Setting::Instance()->IsShowNodes()) {
            // jump to server
            for (size_t k = 0; k < _servers.size(); ++k) {
                if (k != prev)
                    toping.push_back(k);
            }
        } else {
            // jump to hub
            int prevhub = HubIxFromSrvName(GetSrv(prev).name);
            for (size_t k = 0; k < _hubs.size(); ++k) {
                int ixsrv = ServerIdFromHubId(k);
                if (ixsrv != prev) {
                    if (prevhub < 0)
                        toping.push_back(ixsrv);
                    else {
                        if (prevhub != k)
                            toping.push_back(ixsrv);
                    }
                }
            }
        }

        std::vector<int> pings = GetPings(toping);      // from cache; do not wait for pings; return vec of the same size

        IUVec ping_ix;
        for (size_t k = 0; k < toping.size(); ++k) {
            if (pings.at(k) > -1)
                ping_ix.push_back(IUPair(pings.at(k), toping.at(k)));
        }

        if (!ping_ix.empty()) {
            std::sort(ping_ix.begin(), ping_ix.end(), PCmp);
            int num = Setting::Instance()->IsShowNodes() ? 20 : 6;      // pick this many from the top
            if (num >= ping_ix.size())
                num = ping_ix.size();
            int offset = rand() % num;
            srv = ping_ix.at(offset).second;
        }

        // pings can be unavailable
        if (srv < 0) {
            // just pick random
            if (!toping.empty()) {
                srv = toping.at(rand() % toping.size());
            } else {
                if (Setting::Instance()->IsShowNodes()) {
                    if (!_servers.empty())
                        srv = rand() % _servers.size();
                } else {
                    if (!_hubs.empty()) {
                        int h = rand() % _hubs.size();
                        srv = ServerIdFromHubId(h);
                    }
                }
            }
        }
    }
//log::logt("SrvToJump() returns " + QString::number(srv));
    return srv;
}

void AuthManager::jump()
{
    // TODO: -2 update lists
    int srv = getServerToJump();              // except current srv/hub
    if (srv > -1) {
// TODO: -0             SetNewIp("");
        Scr_Map::Instance()->SetServer(srv);
        OpenvpnManager::instance()->start();               // contains stop
    }
}

uint64_t AuthManager::GetRnd64()
{
    DoSeed();
    uint64_t v = 0
                 | ((uint64_t)rand() << 49)
                 | ((uint64_t)rand() << 34)
                 | ((uint64_t)rand() << 19)
                 | ((uint64_t)rand() & 0xf);
    return v;
}

void AuthManager::processOldIP()
{
    log::logt("ProcessOldIpHttp() attempt " + QString::number(_ip_attempt_count));
    QString ip;
    bool err = true;
    if (_reply_IP->error() != QNetworkReply::NoError) {
        log::logt(_reply_IP->errorString());
    } else {
        QByteArray ba = _reply_IP->readAll();
        if (ba.isEmpty()) {
            log::logt("Cannot get old IP address. Server response is empty.");
        } else {
            QString s(ba);
            int p[3];
            int t = 0;
            bool ok = true;
            for (size_t k = 0; k < 3; ++k) {
                p[k] = s.indexOf('.', t);
                if (p[k] < 0) {
                    ok = false;
                    break;
                }
                t = p[k] + 1;
            }
            if (ok) {
                if (s.length() >= QString("2.2.2.2").length()
                        && s.length() <= QString("123.123.123.123").length()) {
                    ip = s;
                    err = false;
                }
            }
        }
    }

    if (err) {
        log::logt("ProcessOldIpHttp() attempt " + QString::number(_ip_attempt_count) + " fails");
        if (_ip_attempt_count < 4)
            getOldIP();
        else
            log::logt("ProcessOldIpHttp() conceide at attempt " + QString::number(_ip_attempt_count));
    } else {
        log::logt("Determined old IP:  " + ip);
        _oldip = ip;
        // try to push value (if Scr_Connect was constructed yet)
        emit oldIpLoaded(_oldip);
    }
}

void AuthManager::ForwardPorts()
{
    UVec ports = Setting::Instance()->ForwardPorts();
    if (!ports.empty()) {
        _forwarder.reset(new PortForwarder(ports, _nam, _aclogin, _acnpsw));
        _forwarder->StartFirst();
        for (size_t k = 0, sz = ports.size(); k < sz; ++k) {

            ;
        }
    }
}

void AuthManager::loginFinished()
{
    QString message = processServersXml();
    if (message.isEmpty())
        emit loginCompleted();
    else
        emit loginError(message);
}


