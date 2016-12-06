#ifndef OSSPECIFIC_H
#define OSSPECIFIC_H

#include <memory>
#include <stdint.h>

#include <QString>
#include <QProcess>

class OsSpecific
{
public:
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }
    static OsSpecific * Instance();
    static void Cleanup()
    {
        if (_inst.get() != NULL) delete _inst.release();
    }
    ~OsSpecific();


    void ExecAsRoot(const QString & cmd, const QStringList & argv);	// throw std::exception on error
#ifndef Q_OS_WIN
    void SetOwnerRoot(const QString & pfn);			// throw std::exception on error
#endif

    // check  and set rights for all the needed files; {chmod, chown, chgrp};
    // the only place requires local elevated rights
    // throw std::exception on error
    void SetRights();

    void SetStartup(bool b);		// Auto-launch app on startup

    int Ping(const QString & adr);			// -1 on error / unavailable
    void StartPing(QProcess & pr, const QString & adr);		// pr must have already connected finished() signal
    int ExtractPing(QProcess & pr);		// exract ping value from pr's stdout; -1 on error / unavailable

    void SetIPv6(bool enable);		// throw std::exception on error
    bool IPv6();	// test OS and return enabled	// throw std::exception on error
    bool HasInsecureWifi();
    void FixDnsLeak();
    void NetDown();

    void EnableTap();	// for Windows enumerate interfaces and for all TAP force call Enable

    const QString IconDisconnected() const;
    const QString IconConnecting() const;
    const QString IconConnected() const;

    const QString IconDisconnected_Selected() const;
    const QString IconConnecting_Selected() const;
    const QString IconConnected_Selected() const;

    const char * IsRunningCmd();

    bool IsNetdown()
    {
        return _netdown;
    }
    void SetNetdown(bool b)
    {
        _netdown = b;
    }

    QString RunFastCmd(const char * cmd, uint16_t ms = 500);			// returns stdout
    QString RunFastCmd(const QString & cmd, uint16_t ms = 500);

    void RunObfs(const QString & srv, const QString & port, const QString & local_port);
    bool IsObfsRunning();
    void StopObfs();
    bool IsObfsInstalled();

    void InstallObfs();

#ifdef Q_OS_MAC
    bool isDark() const;
#endif
private:
    OsSpecific();
    static std::auto_ptr<OsSpecific> _inst;
    std::auto_ptr<QProcess> _obfs;

    void SetChmod(const char * sflags, const QString & pfn);		// flags in form 04555 - will by parsed in both 8- and 16-based
    void SetChown(const QString & pfn);

    bool IsOwnerRoot(const QString & pfn);

    void ReleaseRights();

    const QString & GetCmd();			// both for StartPing()
    QStringList FormatArgs(const QString & adr);
    bool _netdown;

};

#endif // OSSPECIFIC_H
