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

std::auto_ptr<AuthManager> AuthManager::mInstance;
AuthManager * AuthManager::instance()
{
    if (!mInstance.get())
        mInstance.reset(new AuthManager());
    return mInstance.get();
}

bool AuthManager::exists()
{
    return (mInstance.get() != NULL);
}

void AuthManager::cleanup()
{
    if (mInstance.get() != NULL)
        delete mInstance.release();
}

AuthManager::AuthManager()
    :mLoggedIn(false),
    mCancellingLogin(false),
    mSeeded(false),
    mIPAttemptCount(0)
{
    connect(OpenvpnManager::instance(), SIGNAL(gotNewIp(QString)),
            this, SLOT(setNewIp(QString)));
}

AuthManager::~AuthManager()
{
    for (size_t k = 0; k < mWorkers.size(); ++k) {
        if (mTimers.at(k) != NULL) {
            mTimers.at(k)->stop();
            delete mTimers.at(k);
        }
        /*              if (_workers.at(k) != NULL && _waiters.at(k) != NULL)
                        {
                                SjMainWindow * m = SjMainWindow::Instance();
                        }*/
        if (mWorkers.at(k) != NULL) {
            if (mWorkers.at(k)->state() != QProcess::NotRunning)
                mWorkers.at(k)->terminate();
            mWorkers.at(k)->deleteLater();
        }
        if (mWaiters.at(k) != NULL)
            delete mWaiters.at(k);
    }

    if (mOldIPThread.get() != NULL) {
        if (mOldIPThread->isRunning())
            mOldIPThread->terminate();
        delete mOldIPThread.get();
    }

    if (mPortForwarderThread.get() != NULL) {
        // TODO: -1 terminate connections; disconnect signals
        delete mPortForwarderThread.release();
    }

    // TODO: -0 terminate Network Manager
//      _nam
}

bool AuthManager::loggedIn()
{
    // TODO: -0 not implemented
    return mLoggedIn;
}

void AuthManager::login(const QString & name, const QString & password)
{
    mAccountLogin = name;
    mAccountPassword = password;

    mVPNLogin.clear();
    mVPNPassword.clear();                    // TODO: -2 secure clear
    mLoggedIn = false;
    mCancellingLogin = false;
    log::logt("Starting login with name '" + QUrl::toPercentEncoding(name, "", "") + "'");

    mReply.reset(mNAM.get(BuildRequest(QUrl("https://proxy.sh/access.php?u=" + QUrl::toPercentEncoding(name, "", "") + "&p=" + QUrl::toPercentEncoding(password, "", "")))));
    connect(mReply.get(), SIGNAL(finished()), this, SLOT(loginFinished()));
}

void AuthManager::cancel()
{
    mCancellingLogin = true;
    if(NULL != mReply.get()) {
        mReply->abort();
    }
}

void AuthManager::logout()
{
    cancel();
    mLoggedIn = false;
    mVPNLogin.clear();
    mVPNPassword.clear();
    mAccountLogin.clear();
    mAccountPassword.clear();            // TODO: -2 secure clear

    emit logoutCompleted();
}

const QString &AuthManager::accountName()
{
    return mAccountLogin;    // TODO: -1 check: still valid, synchro with the main wnd
}

const QString &AuthManager::accountPassword()
{
    return mAccountPassword;
}

const QString &AuthManager::VPNName()
{
    return mVPNLogin;    // TODO: -1 check: still valid
}

const QString &AuthManager::VPNPassword()
{
    return mVPNPassword;
}

const QString &AuthManager::newIP()
{
    return mNewIP;
}

const QString &AuthManager::oldIP()
{
    return mOldIP;
}

AServer AuthManager::getServer(int id)
{
    AServer s;
    //assert(id > -1);
    if (id > -1 && id < mServers.size()) {
        s = mServers.at(id);
    } else {
        log::logt("getServer called with id " + QString::number(id));
    }
    return s;
}

AServer AuthManager::getHub(int idhub)
{
    int idsrv = -1;
    if (idhub > -1 && idhub < mHubs.size())
        idsrv = serverIdFromHubId(idhub);
    return getServer(idsrv);
}

void AuthManager::setNewIp(const QString & ip)
{
    static const QString self = "127.0.0.1";
    if (ip != self) {
        mNewIP = ip;
        emit newIpLoaded(ip);
    }
}

const std::vector<size_t> & AuthManager::currentEncryptionServers()
{
    int enc = Setting::encryption();
    return mServerIds[enc];
}

const std::vector<size_t> & AuthManager::currentEncryptionHubs()
{
    if (!mServers.isEmpty() && mHubs.empty()) {
        for (int k = 0; k < mServers.size(); ++k) {
            if (mServers.at(k).name.contains("Hub", Qt::CaseInsensitive)) {
                mHubs.append(mServers.at(k));
                mHubToServer.push_back(k);                      // the same as _hub_ids[0]
                mHubIds[0].push_back(k);
                mServerIdToHubId.insert(IIMap::value_type(k, mHubs.size() - 1));
                std::string cleared = flag::ClearName(mServers[k].name).toStdString();          // QString cleared = flag::ClearName(_servers[k].name);
                mHubClearedId.insert(std::make_pair(cleared, mHubs.size() - 1));
            }
        }
    }
    int enc = Setting::encryption();
    return mHubIds[enc];
}

int AuthManager::hubidForServerNode(size_t srv)
{
    int hub = -1;
    if (srv > -1 && srv < mServers.size()) {
        const AServer & rs = mServers.at(srv);
        std::string cleared = flag::ClearName(rs.name).toStdString();       // QString cleared = flag::ClearName(rs.name);
        std::map<std::string, size_t>::iterator it = mHubClearedId.find(cleared);
        if (it != mHubClearedId.end())
            hub = (int)(*it).second;
    } else {
        log::logt("Hub ID For Server Node " + QString::number(srv) + " requested but out of bounds");
    }
    return hub;
}

const std::vector<std::pair<bool, int> > & AuthManager::getLevel0()
{
    prepareLevels();
    return mLevel0;
}

void AuthManager::prepareLevels()
{
    log::logt("prepareLevels called");
    // TODO: -1 special hub for boost
    if (!mServers.isEmpty() && mLevel0.empty()) {
        const std::vector<size_t> & h = currentEncryptionHubs();
        std::set<int> hub_srvids;
        for (size_t k =0; k < h.size(); ++k) {
            int srv = h.at(k);  //ServerIdFromHubId(k);
            hub_srvids.insert(srv);
            std::vector<int> v;
            v.push_back(srv);
            mLevel1.insert(std::make_pair(k, v));
        }

        const std::vector<size_t> & sr = currentEncryptionServers();
        for (size_t k = 0; k < sr.size(); ++k) {
            int srv = sr.at(k);
            std::set<int>::iterator it = hub_srvids.find(srv);
            if (it != hub_srvids.end()) {
                mLevel0.push_back(std::make_pair(true, hubIdFromServerId(*it)));        // aready at lvl 1
            } else {    // just server: lvl0 or lvl1
                int chub = hubidForServerNode(k);
                if (chub > -1) {
                    std::map<int, std::vector<int> >::iterator it2 = mLevel1.find(chub);
                    if (it2 != mLevel1.end()) {
                        std::vector<int> & rv = (*it2).second;
                        rv.push_back(k);
                    }
                } else {
                    mLevel0.push_back(std::make_pair(false, k));
                }
            }
        }
    }
}

const std::vector<int> & AuthManager::getLevel1(size_t hub)
{
    prepareLevels();
    std::map<int, std::vector<int> >::iterator it = mLevel1.find(hub);
    if (it == mLevel1.end())
        return mFake;
    else
        return (*it).second;
}

size_t AuthManager::serverIdFromHubId(size_t ixHub)
{
    if (ixHub > -1 && ixHub < mHubToServer.size())
        return mHubToServer.at(ixHub);
    return -1;
}

int AuthManager::hubIxFromServerName(const QString & srv)
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
        int ix2 = serverIxFromName(s3);    // try 'Canada' + ' Hub'
        if (ix2 > -1)   // convert id into hub id
            ix = hubIdFromServerId(ix2);
    }
    return ix;
}

int AuthManager::hubIdFromServerId(int ixsrv)
{
    int hub = -1;
    if (ixsrv > -1) {
        IIMap::const_iterator ci = mServerIdToHubId.find(ixsrv);
        if (ci != mServerIdToHubId.end())
            hub = (*ci).second;
    }
    return hub;
}

int AuthManager::serverIxFromName(const QString & srv)
{
    int ix = -1;
    if (mServerNameToId.empty() && !mServers.isEmpty()) {
        for (size_t k = 0, sz = mServers.size(); k < sz; ++k)
            mServerNameToId.insert(SIMap::value_type(mServers.at(k).name.toStdString(), k));
    }
    SIMap::iterator it = mServerNameToId.find(srv.toStdString());
    if (it != mServerNameToId.end())
        ix = (*it).second;
    return ix;
}

int AuthManager::pingFromServerIx(int srv)
{
    int pn = -1;
    if (srv > -1 && srv < mPings.size())
        pn =mPings.at(srv);
    return pn;
}

void AuthManager::clearServerLists()
{
    mHubs.clear();
    mLevel0.clear();
    mLevel1.clear();
    mHubToServer.clear();
    mServerIdToHubId.clear();
    mServerNameToId.clear();
    mServers.clear();
    mPings.clear();
}

void AuthManager::clearReply()
{
    if (mReply.get() != NULL) {
        mReply->abort();
        mReply->deleteLater();
        mReply.release();
    }
}

void AuthManager::getEccServerNames()
{
    clearReply();
    // https://api.proxy.sh/safejumper/get_ecc/name
    mReply.reset(AuthManager::instance()->mNAM.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/get_ecc/name"))));
    connect(mReply.get(), SIGNAL(finished()),
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
        mServerIds[ENCRYPTION_ECCXOR].clear();
        mServerIds[ENCRYPTION_ECCXOR].assign(mServerIds[ENCRYPTION_ECC].begin(), mServerIds[ENCRYPTION_ECC].end());
        forceRepopulation(ENCRYPTION_ECC);
        forceRepopulation(ENCRYPTION_ECCXOR);
        if (Setting::encryption() != ENCRYPTION_RSA)
            Setting::instance()->loadServer();
    }
}

void AuthManager::getObfsServerNames()
{
    clearReply();
    // https://api.proxy.sh/safejumper/get_obfs/name
    mReply.reset(AuthManager::instance()->mNAM.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/get_obfs/name"))));
    connect(mReply.get(), SIGNAL(finished()),
            this, SLOT(processObfsServerNamesXml()));
}

void AuthManager::processObfsServerNamesXml()
{
    QString message;
    bool err = processServerNamesForEncryptionType(ENCRYPTION_TOR_OBFS2, message);

    if (!err) {
        // clone ECC nodes into ECC+XOR
        // TODO: -2 is there a specific for ECC+XOR  API page?
        mServerIds[ENCRYPTION_TOR_OBFS3].clear();
        mServerIds[ENCRYPTION_TOR_OBFS3].assign(mServerIds[ENCRYPTION_TOR_OBFS2].begin(), mServerIds[ENCRYPTION_TOR_OBFS2].end());
        mServerIds[ENCRYPTION_TOR_SCRAMBLESUIT].clear();
        mServerIds[ENCRYPTION_TOR_SCRAMBLESUIT].assign(mServerIds[ENCRYPTION_TOR_OBFS2].begin(), mServerIds[ENCRYPTION_TOR_OBFS2].end());
        forceRepopulation(ENCRYPTION_TOR_OBFS2);
        forceRepopulation(ENCRYPTION_TOR_OBFS3);
        forceRepopulation(ENCRYPTION_TOR_SCRAMBLESUIT);
        if (Setting::encryption() != ENCRYPTION_RSA)
            Setting::instance()->loadServer();
    }

    // do not get obfs addresses: proceed
    getEccServerNames();

    if (!err)
        forceRepopulation(ENCRYPTION_TOR_OBFS2);

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
    mReply.reset(AuthManager::instance()->mNAM.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/account_type/"
                          + QUrl::toPercentEncoding(AuthManager::instance()->VPNName(), "", "")
                          + "/" + QUrl::toPercentEncoding(AuthManager::instance()->VPNPassword(), "", "")))));
    connect(mReply.get(), SIGNAL(finished()),
            this, SLOT(processAccountTypeXml()));
}

void AuthManager::getExpirationDate()
{
    clearReply();
    // https://api.proxy.sh/safejumper/expire_date/VPNusername/VPNpassword
    mReply.reset(AuthManager::instance()->mNAM.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/expire_date/"
                          + QUrl::toPercentEncoding(AuthManager::instance()->VPNName(), "", "")
                          + "/" + QUrl::toPercentEncoding(AuthManager::instance()->VPNPassword(), "", "")))));
    connect(mReply.get(), SIGNAL(finished()),
            this, SLOT(processExpirationXml()));
}

void AuthManager::checkUpdates()
{
    QString us(SJ_UPDATE_URL);
    if (!us.isEmpty()) {
        mUpdateReply.reset(AuthManager::instance()->mNAM.get(BuildRequest(QUrl(us))));
        connect(mUpdateReply.get(), SIGNAL(finished()),
                this, SLOT(processUpdatesXml()));
    }
}

void AuthManager::getOldIP()
{
    ++mIPAttemptCount;
    log::logt("StartDwnl_OldIp() attempt " + QString::number(mIPAttemptCount));
    static const QString us = "https://proxy.sh/ip.php";
    mIPReply.reset(AuthManager::instance()->mNAM.get(BuildRequest(QUrl(us))));
    connect(mIPReply.get(), SIGNAL(finished()),
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
    mReply.reset(AuthManager::instance()->mNAM.get(BuildRequest(
                     QUrl("https://api.proxy.sh/safejumper/get_dns"))));
    connect(mReply.get(), SIGNAL(finished()),
            this, SLOT(processDnsXml()));
}

void AuthManager::processAccountTypeXml()
{
    QString message;
    if (mReply->error() != QNetworkReply::NoError) {
        log::logt(mReply->errorString());
        return;
    }
    QByteArray ba = mReply->readAll();
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
    if (mReply->error() != QNetworkReply::NoError) {
        log::logt(mReply->errorString());
        return;
    }
    QByteArray ba = mReply->readAll();
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
    if (mReply->error() != QNetworkReply::NoError) {
        log::logt(mReply->errorString());
        return;
    }
    QByteArray ba = mReply->readAll();
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
        Setting::instance()->setDefaultDNS(dns[0], dns[1]);

    clearReply();               // TODO: -2 further processing here
}

void AuthManager::processUpdatesXml()
{
    if (mUpdateReply->error() != QNetworkReply::NoError) {
        log::logt(mUpdateReply->errorString());
        return;
    }
    QByteArray ba = mUpdateReply->readAll();

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
            Setting::instance()->updateMessageShown();
            if (result == QDialog::Accepted)
                launchUpdateUrl();           // TODO: -2 auto update self
        }
    }
}

bool AuthManager::processServerNamesForEncryptionType(int enc, QString & out_msg)
{
    out_msg.clear();
    QStringList names = extractNames(out_msg);

    if (out_msg.isEmpty() && !names.isEmpty()) {
       populateServerIdsFromNames(names, mServerIds[enc]);
    }

    return !out_msg.isEmpty();
}

void AuthManager::forceRepopulation(int enc)
{
    // force update of locations: if needed, previously empy
    if (Setting::encryption() == enc) {
        if (Setting::exists() && Scr_Map::IsExists()) {
            Scr_Map::Instance()->RePopulateLocations();
            // TODO: -0 LoadServer()
        }
    }
}

QStringList AuthManager::extractNames(QString & out_msg)
{
    QStringList names;
    if (mReply->error() != QNetworkReply::NoError) {
        log::logt("Network error:" + mReply->errorString());
        out_msg = "Network error: " + mReply->errorString();
    } else {
        QByteArray ba = mReply->readAll();

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
    for (size_t j = 0, ses = mServers.size(); j < ses; ++j) {
        if (names.contains(mServers.at(j).name)) {
            paid.push_back(j);
        }
    }
    std::sort(paid.begin(), paid.end());
    found.swap(paid);
}

QString AuthManager::processServersXml()
{
    QString message;
    clearServerLists();
    mVPNLogin.clear();
    mVPNPassword.clear();
    if (mReply->error() != QNetworkReply::NoError) {
        mLoggedIn = false;
        return mReply->errorString();
    } else {
        QByteArray ba = mReply->readAll();
        if (ba.isEmpty()) {
            mLoggedIn = false;
            return "Cannot log in with this name and password pair. Server response is empty.";
        }

        // parse XML response
        QDomDocument doc;
        QString login, psw;
        if (!doc.setContent(QString(ba), &message)) {
            mLoggedIn = false;
            return "Error parsing server XML response\n" + message;
        } else {
            QDomNodeList nlLogin = doc.elementsByTagName("username");
            if (nlLogin.size() > 0) {
                QDomNode n = nlLogin.item(0);
                login = n.toElement().text();
            } else {
                mLoggedIn = false;
                return "Missing credentials";
            }
        }
        QDomNodeList nl = doc.elementsByTagName("password");
        if (nl.size() > 0) {
            QDomNode n = nl.item(0);
            psw = n.toElement().text();
        } else {
            mLoggedIn = false;
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
                mServers.append(s2);
                mServerIds[0].push_back(mServers.size() - 1);
            }
            mVPNLogin = login;
            mVPNPassword = psw;
            mLoggedIn = true;
        } else {
            mLoggedIn = false;
            return "Incorrect credentials. Make sure to use your VPN credentials and not your email.";
        }
    }

    // force hubs
    const std::vector<size_t> & hr = AuthManager::currentEncryptionHubs();
    if (hr.empty())
        log::logt("Cannot parse hubs");

    getObfsServerNames();
//  StartDwnl_AccType();

    if (!Setting::instance()->testing())
        pingAllServers(); // No need to ping servers when in testing mode

    return message;
}

void AuthManager::pingAllServers()
{
    log::logt("pingAllServers called");
    // Fill _pinks with as many -1 entries as there are servers
    if (mPings.empty())
        mPings.assign(mServers.size(), -1);
    if (mPings.size() < mServers.size()) {
        for (size_t k = 0, n = mServers.size() - mPings.size(); k < n; ++k)
            mPings.push_back(-1);
    }
    for (size_t k = 0; k < mServers.size(); ++k)
        mToPing.push(k);

    if (mWorkers.empty()) {
        mInProgress.assign(PINGWORKERS_NUM, 0);
        LoginWindow * m = LoginWindow::Instance();
        for (size_t k = 0; k < PINGWORKERS_NUM; ++k) {
            mWorkers.push_back(NULL);
            mWaiters.push_back(new PingWaiter(k, m));
            mTimers.push_back(new QTimer());
        }
    }

    mPingsLoaded = false;
    for (size_t k = 0; k < mWorkers.size() && !mToPing.empty(); ++k)
        startWorker(k);
}

void AuthManager::startWorker(size_t id)
{
    log::logt("startWorker called with id " + QString::number(id));
    if (!mToPing.empty()) {
        size_t srv = mToPing.front();
        mToPing.pop();

        //log::logt(QString("Pop srv ") + QString::number(srv));

        mInProgress.at(id) = srv;
        LoginWindow * m = LoginWindow::Instance();

        if (mWorkers.at(id) !=NULL) {
            m->disconnect(mWorkers.at(id), SIGNAL(finished(int,QProcess::ExitStatus)),
                          mWaiters.at(id), SLOT(PingFinished(int,QProcess::ExitStatus)));
            m->disconnect(mWorkers.at(id), SIGNAL(error(QProcess::ProcessError)),
                          mWaiters.at(id), SLOT(PingError(QProcess::ProcessError)));
            if (mWorkers.at(id)->state() != QProcess::NotRunning)
                mWorkers.at(id)->terminate();
        }
        std::auto_ptr<QProcess> raii(mWorkers.at(id));          // force delete if any
        mWorkers[id] = new QProcess(m);
        m->connect(mWorkers.at(id), SIGNAL(finished(int,QProcess::ExitStatus)), mWaiters.at(id), SLOT(PingFinished(int,QProcess::ExitStatus)));
        m->connect(mWorkers.at(id), SIGNAL(error(QProcess::ProcessError)), mWaiters.at(id), SLOT(PingError(QProcess::ProcessError)));
        OsSpecific::instance()->startPing(*mWorkers.at(id), mServers.at(srv).address);
        m->connect(mTimers.at(id), SIGNAL(timeout()), mWaiters.at(id), SLOT(Timer_Terminate()));
        mTimers.at(id)->setSingleShot(true);
        mTimers.at(id)->start(PINGWORKER_MAX_TIMEOUT);
    } else {
        if (!mPingsLoaded) {
            mPingsLoaded = true;
            Scr_Map::Instance()->RePopulateLocations();         // load pings
        }
    }
}

void AuthManager::pingComplete(size_t idWaiter)
{
    log::logt("pingComplete called with id " + QString::number(idWaiter));
    mTimers.at(idWaiter)->stop();
    int p = OsSpecific::instance()->extractPing(*mWorkers.at(idWaiter));
//      log::logt(_servers.at(_inprogress.at(idWaiter)).address + " Got ping " + QString::number(p));
    mPings.at(mInProgress.at(idWaiter)) = p;
    startWorker(idWaiter);
}

void AuthManager::pingError(size_t idWaiter)
{
    log::logt("pingError called with id " + QString::number(idWaiter));
    mTimers.at(idWaiter)->stop();
    int p = OsSpecific::instance()->extractPing(*mWorkers.at(idWaiter));
//      log::logt(_servers.at(_inprogress.at(idWaiter)).address + " ping process error, extracted ping: " + QString::number(p));
    mPings.at(mInProgress.at(idWaiter)) = p;
    startWorker(idWaiter);
}

void AuthManager::pingTerminated(size_t idWaiter)
{
    log::logt("pingTerminated called with id " + QString::number(idWaiter));
    mWorkers.at(idWaiter)->terminate();
    int p = OsSpecific::instance()->extractPing(*mWorkers.at(idWaiter));
//      log::logt(_servers.at(_inprogress.at(idWaiter)).address + " ping process terminated, extracted ping: " + QString::number(p));
    mPings.at(mInProgress.at(idWaiter)) = p;
    startWorker(idWaiter);
}

void AuthManager::seed()
{
    if (!mSeeded) {
        srand(time(NULL));
        mSeeded = true;
    }
}

std::vector<int> AuthManager::getPings(const std::vector<size_t> & toping)
{
    std::vector<int> v;
    v.assign(toping.size(), -1);
    if (mPings.empty())
        log::logt("GetPings(): Empty pings collection");
    else {
        for (size_t k = 0; k < toping.size(); ++k) {
            if (toping.at(k) >= mPings.size())
                log::logt("GetPings(): Server id greater than size of pings coll");
            else
                v.at(k) = mPings.at(toping.at(k));
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
    log::logt("getServerToJump called");
    if (mServers.isEmpty()) {
        log::logt("Server list not loaded, so using -1");
        return -1;
    }
    int srv = -1;
    int prev = Scr_Map::Instance()->CurrSrv();
    log::logt("Previous server is " + QString::number(prev));
    std::vector<size_t> toping;     // ix inside mServers
    int enc = Setting::instance()->encryption();
    if (Setting::instance()->showNodes()) {
        log::logt("showNodes is set, so getting pings of all servers");
        // jump to server
        for (size_t k = 0; k < mServerIds[enc].size(); ++k) {
            if (mServerIds[enc].at(k) != prev)
                toping.push_back(mServerIds[enc].at(k));
        }
    } else {
        // jump to hub
        int prevhub = hubIxFromServerName(getServer(prev).name);
        for (size_t k = 0; k < mHubIds[enc].size(); ++k) {
            int ixsrv = serverIdFromHubId(mHubIds[enc].at(k));
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
    log::logt("getServerToJump pings list is " + QString::number(toping.size()));

    std::vector<int> pings = getPings(toping);      // from cache; do not wait for pings; return vec of the same size

    IUVec ping_ix;
    for (size_t k = 0; k < toping.size(); ++k) {
        if (pings.at(k) > -1)
            ping_ix.push_back(IUPair(pings.at(k), toping.at(k)));
    }

    if (!ping_ix.empty()) {
        std::sort(ping_ix.begin(), ping_ix.end(), PCmp);
        int num = Setting::instance()->showNodes() ? 20 : 6;      // pick this many from the top
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
            if (Setting::instance()->showNodes()) {
                if (!mServers.isEmpty())
                    srv = rand() % mServers.size();
            } else {
                if (!mHubs.empty()) {
                    int h = rand() % mHubs.size();
                    srv = serverIdFromHubId(h);
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

uint64_t AuthManager::getRandom64()
{
    seed();
    uint64_t v = 0
                 | ((uint64_t)rand() << 49)
                 | ((uint64_t)rand() << 34)
                 | ((uint64_t)rand() << 19)
                 | ((uint64_t)rand() & 0xf);
    return v;
}

void AuthManager::processOldIP()
{
    log::logt("ProcessOldIpHttp() attempt " + QString::number(mIPAttemptCount));
    QString ip;
    bool err = true;
    if (mIPReply->error() != QNetworkReply::NoError) {
        log::logt(mIPReply->errorString());
    } else {
        QByteArray ba = mIPReply->readAll();
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
        log::logt("ProcessOldIpHttp() attempt " + QString::number(mIPAttemptCount) + " fails");
        if (mIPAttemptCount < 4)
            getOldIP();
        else
            log::logt("ProcessOldIpHttp() conceide at attempt " + QString::number(mIPAttemptCount));
    } else {
        log::logt("Determined old IP:  " + ip);
        mOldIP = ip;
        // try to push value (if Scr_Connect was constructed yet)
        emit oldIpLoaded(mOldIP);
    }
}

void AuthManager::forwardPorts()
{
    UVec ports = Setting::instance()->forwardPorts();
    if (!ports.empty()) {
        mPortForwarderThread.reset(new PortForwarder(ports, mNAM, mAccountLogin, mAccountPassword));
        mPortForwarderThread->StartFirst();
        for (size_t k = 0, sz = ports.size(); k < sz; ++k) {

            ;
        }
    }
}

void AuthManager::loginFinished()
{
    QString message = processServersXml();
    log::logt("loginFinished called message is " + message);
    if (message.isEmpty())
        emit loginCompleted();
    else
        emit loginError(message);
}


