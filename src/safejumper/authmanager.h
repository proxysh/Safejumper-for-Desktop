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

#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <memory>
#include <queue>
#include <stdint.h>

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QProcess>

#include "common.h"
#include "thread_oldip.h"
#include "pingwaiter.h"
#include "portforwarder.h"

#define PINGWORKERS_NUM 2
#define PINGWORKER_MAX_TIMEOUT 2000

#ifndef uint64_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <time.h>
#include <errno.h>

#endif		// #ifndef uint64_t

class AuthManager: public QObject
{
    Q_OBJECT
public:
    static AuthManager * instance();
    static bool exists();
    static void cleanup();

    ~AuthManager();

    bool loggedIn();

    void login(const QString & name, const QString & password);
    void cancel();
    void logout();

    const QString & accountName();
    const QString & accountPassword();
    const QString & VPNName();
    const QString & VPNPassword();

    const QString & newIP();
    const QString & oldIP();

    const std::vector<size_t> & currentEncryptionServers();		// return IDs of servers inside _servers available for this encryption
    const std::vector<size_t> & currentEncryptionHubs();					// return IDs of habs inside _servers

    const std::vector<std::pair<bool, int> > & getLevel0();		// <is hub, hub id / srv id>
    const std::vector<int> & getLevel1(size_t hub);					// for given hub id all the server ids, including hub entry itself

    size_t serverIdFromHubId(size_t ixHub);
    AServer getServer(int id);	 // on -1 returns empty
    int hubIxFromServerName(const QString & srv);	  // -1 if not found
    int hubIdFromServerId(int ixsrv);			   // -1 if ixsrv not a hub
    int serverIxFromName(const QString & srv);		 // -1 if not found
    AServer getHub(int idhub);

    int pingFromServerIx(int srv);

    void jump();

    uint64_t getRandom64();

    void checkUpdates();		// use own reply; can download in parallel with others; executed by main window at start regardless other actions
    void getOldIP();

    void pingComplete(size_t idWaiter);
    void pingError(size_t idWaiter);
    void pingTerminated(size_t idWaiter);

    int getServerToJump();		// except current Scr_Map::Instance()->CurrSrv()

    void forwardPorts();

public slots:
    void setNewIp(const QString & ip);

signals:
    void loginCompleted();
    void loginError(QString message);
    void untilLoaded(QString until);
    void amountLoaded(QString amount);
    void emailLoaded(QString email);
    void oldIpLoaded(QString oldIp);
    void newIpLoaded(QString newIp);

private slots:
    void loginFinished();
    void processObfsServerNamesXml();
    void processEccServerNamesXml();
    void processAccountTypeXml();
    void processExpirationXml();
    void processDnsXml();

    void processUpdatesXml();
    void processOldIP();
private:
    AuthManager();
    QString processServersXml();	// true = ok, empty msg on ok
    bool processServerNamesForEncryptionType(int enc, QString & out_msg);
    void populateServerIdsFromNames(QStringList names, std::vector<size_t> & found);		// for _obfs_names lookup respective server ix in _servers
    QStringList extractNames(QString & out_msg);
    void pingAllServers();

    bool mLoggedIn;
    bool mCancellingLogin;
    static std::auto_ptr<AuthManager> mInstance;

    void clearServerLists();
    bool mSeeded;
    void seed();

    std::vector<AServer> mServers;
    std::vector<size_t> mServerIds[ENCRYPTION_COUNT];		// IDs inside _servers available for each encryption
    std::vector<AServer> mHubs;
    std::vector<size_t> mHubIds[ENCRYPTION_COUNT];		// IDs of hubs inside _servers available for each encryption		// _hub_ids[0] the same as _HubToServer
    std::vector<std::pair<bool, int> > mLevel0;		// <is hub, hub id / srv id>
    std::map<int, std::vector<int> > mLevel1;		// <hub id, <srv ids, including srv id of hub entry> >
    std::vector<int> mFake;
    void prepareLevels();
    int hubidForServerNode(size_t srv);					// -1 if cannot find hub for this srv
    std::map<std::string, size_t> mHubClearedId;	//std::map<QString, size_t> _HubClearedId;		// <hub cleared name (w/o ' Hub'), its hub id>
    std::vector<size_t> mHubToServer;	   // id of hub inside _servers
    IIMap mServerIdToHubId;
    SIMap mServerNameToId;

//	std::vector<QString> _obfs_names;	// U.S. California 3
//	std::vector<int> _obfs_srv;				// id in _servers ; -1 if cannot find
    //std::vector<int> _obfs_srv_available;	// id in _servers of servers available for this user (inside paid set)
//	std::set<int> _obfs_enabled_srvs;		// for lookup
    std::vector<QString> mObfsAddress;		// 64.110.129.100 ; or boost-sg.proxy.sh

    QString mAccountLogin;
    QString mAccountPassword;
    QString mVPNLogin;
    QString mVPNPassword;

    QString mNewIP;
    QString mOldIP;
    QString mAccountName;
    QString mEmail;
    QString mAmount;
    QString mUntil;

    QNetworkAccessManager mNAM;
    std::auto_ptr<QNetworkReply> mReply;
    std::auto_ptr<QNetworkReply> mIPReply;
    int mIPAttemptCount;
    void clearReply();
    std::auto_ptr<QNetworkReply> mUpdateReply;

    std::vector<int> mPings;		// ping for each server; -1 on err
    std::vector<int> getPings(const std::vector<size_t> & toping);	// from _pings; do not wait for pings; return vec of the same size
    std::queue<size_t> mToPing;				// id inside _servers
    bool mPingsLoaded;

    std::vector<QProcess *> mWorkers;		// 3 vectors of the same size WORKERS_NUM
    std::vector<PingWaiter *> mWaiters;
    std::vector<size_t> mInProgress;			// id inside _servers
    std::vector<QTimer *>  mTimers;
    void startWorker(size_t id);

    void getAccountType();		// TODO: -2 chain in a more flexible way e.g. queue
    void getExpirationDate();
    void getDns();
    void getObfsServerNames();
    void getEccServerNames();

    void forceRepopulation(int enc);

    std::auto_ptr<Thread_OldIp> mOldIPThread;
    std::auto_ptr<PortForwarder> mPortForwarderThread;
};

#endif // AUTHMANAGER_H
