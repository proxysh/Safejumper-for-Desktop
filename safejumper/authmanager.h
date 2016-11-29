#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <memory>
#include <queue>
#include <stdint.h>

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

#define PINGWORKERS_NUM 16
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

class AuthManager
{
public:
    static AuthManager * Instance();
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }
    static void Cleanup();
    ~AuthManager();

    bool IsLoggedin();

    void DoLogin(const QString & name, const QString & password);
    void CancelLogin();
    void DoLogout();

    const QString & AccName()
    {
        return _aclogin;    // TODO: -1 check: still valid, synchro with the main wnd
    }
    const QString & AccPsw()
    {
        return _acnpsw;
    }
    const QString & VpnName()
    {
        return _vpnlogin;    // TODO: -1 check: still valid
    }
    const QString & VpnPsw()
    {
        return _vpnpsw;
    }

    const QString & NewIp()
    {
        return _newip;
    }
    void SetNewIp(const QString & ip);

    const QString & OldIp()
    {
        return _oldip;
    }
    QNetworkAccessManager & Nam()
    {
        return _nam;
    }

    const std::vector<size_t> & GetAllServers();		// return IDs of servers inside _servers available for this encryption
    const std::vector<size_t> & GetHubs();					// return IDs of habs inside _servers

    const std::vector<std::pair<bool, int> > & GetLvl0();		// <is hub, hub id / srv id>
    const std::vector<int> & GetLvl1(size_t hub);					// for given hub id all the server ids, including hub entry itself

    size_t ServerIdFromHubId(size_t ixHub);
    AServer GetSrv(int id);	 // on -1 returns empty
    bool IsObfs(int id);			// true if server has Obfs proxy
    int HubIxFromSrvName(const QString & srv);	  // -1 if not found
    int HubIdFromItsSrvId(int ixsrv);			   // -1 if ixsrv not a hub
    int SrvIxFromName(const QString & srv);		 // -1 if not found
    AServer GetHub(int idhub);

    int PingFromSrvIx(int srv);

    void Jump();
    void DetermineOldIp();

    uint64_t GetRnd64();

    bool ProcessXml_Servers(QString & out_msg);	// true = ok, empty msg on ok
    bool ProcessXml_ObfsName(QString & out_msg);
    bool ProcessXml_EccName(QString & out_msg);
    bool ProcessAccountXml(QString & out_msg);
    bool ProcessExpireXml(QString & out_msg);
    void ProcessOldIp(QString ip);
    void ProcessOldIpHttp();
    void ProcessDnsXml();
    void ProcessUpdatesXml();

    void StartDwnl_Updates();		// use own reply; can download in parallel with others; executed by main window at start regardless other actions
    void StartDwnl_OldIp();

    void PingComplete(size_t idWaiter);
    void PingErr(size_t idWaiter);
    void PingTerminate(size_t idWaiter);

    int SrvToJump();		// except current Scr_Map::Instance()->CurrSrv()

    void ForwardPorts();

private:
    AuthManager();
    bool _logged;
    bool _CancelLogin;
    static std::auto_ptr<AuthManager> _inst;

    void ClearColls();
    bool _seeded;
    void DoSeed();

    std::vector<AServer> _servers;
    std::vector<size_t> _srv_ids[ENCRYPTION_COUNT];		// IDs inside _servers available for each encryption
    std::vector<AServer> _hubs;
    std::vector<size_t> _hub_ids[ENCRYPTION_COUNT];		// IDs of hubs inside _servers available for each encryption		// _hub_ids[0] the same as _HubToServer
    std::vector<std::pair<bool, int> > _level0;		// <is hub, hub id / srv id>
    std::map<int, std::vector<int> > _level1;		// <hub id, <srv ids, including srv id of hub entry> >
    std::vector<int> _fake;
    void PrepareLevels();
    int HubidForServerNode(size_t srv);					// -1 if cannot find hub for this srv
    std::map<std::string, size_t> _HubClearedId;	//std::map<QString, size_t> _HubClearedId;		// <hub cleared name (w/o ' Hub'), its hub id>
    std::vector<size_t> _HubToServer;	   // id of hub inside _servers
    IIMap _SrvidToHubId;
    SIMap _SrvNameToId;

//	std::vector<QString> _obfs_names;	// U.S. California 3
//	std::vector<int> _obfs_srv;				// id in _servers ; -1 if cannot find
    //std::vector<int> _obfs_srv_available;	// id in _servers of servers available for this user (inside paid set)
//	std::set<int> _obfs_enabled_srvs;		// for lookup
    std::vector<QString> _obfs_addr;		// 64.110.129.100 ; or boost-sg.proxy.sh
    void MatchObfsServers(const std::vector<QString> & names, std::vector<size_t> & found);		// for _obfs_names lookup respective server ix in _servers

    QString _aclogin;
    QString _acnpsw;
    QString _vpnlogin;
    QString _vpnpsw;

    QString _newip;
    QString _oldip;
    QString _accname;
    QString _email;
    QString _amount;
    QString _until;

    QNetworkAccessManager _nam;
    std::auto_ptr<QNetworkReply> _reply;
    std::auto_ptr<QNetworkReply> _reply_IP;
    int _ip_attempt_count;
    void ClearReply();
    std::auto_ptr<QNetworkReply> _reply_update;

    std::vector<int> _pings;		// ping for each server; -1 on err
    std::vector<int> GetPings(const std::vector<size_t> & toping);	// from _pings; do not wait for pings; return vec of the same size
    void PingAll();
    std::queue<size_t> _toping;				// id inside _servers
    bool _pings_loaded;

    std::vector<QProcess *> _workers;		// 3 vectors of the same size WORKERS_NUM
    std::vector<PingWaiter *> _waiters;
    std::vector<size_t> _inprogress;			// id inside _servers
    std::vector<QTimer *>  _timers;
    void StartWorker(size_t id);

    void StartDwnl_AccType();		// TODO: -2 chain in a more flexible way e.g. queue
    void StartDwnl_Until();
    void StartDwnl_Dns();
    void StartDwnl_ObfsName();
    void StartDwnl_EccName();
//	void StartDwnl_EccxorName();

    bool ProcessXml_Names(std::vector<QString> & v, QString & out_msg);
    bool ProcessXml_EncryptionName(int enc, QString & out_msg);
    void ForceRepopulation(int enc);

    std::auto_ptr<Thread_OldIp> _th_oldip;
    std::auto_ptr<PortForwarder> _forwarder;
};

#endif // AUTHMANAGER_H
