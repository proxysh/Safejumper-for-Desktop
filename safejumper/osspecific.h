#ifndef OSSPECIFIC_H
#define OSSPECIFIC_H

#include <memory>

#include <QString>
#include <QProcess>

class OsSpecific
{
public:
	static bool IsExists() { return (_inst.get() != NULL); }
	static OsSpecific * Instance();
	static void Cleanup() { if (_inst.get() != NULL) delete _inst.release();}
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

	void EnableTap();	// for Windows enumerate interfaces and for all TAP force call Enable
	
	const char * IconDisconnected();
	const char * IconConnecting();
	const char * IconConnected();
	
	const char * IconDisconnected_Selected();
	const char * IconConnecting_Selected();
	const char * IconConnected_Selected();
	
	const char * IsRunningCmd();
	
private:
	OsSpecific();
	static std::auto_ptr<OsSpecific> _inst;

	void SetChmod(const char * sflags, const QString & pfn);		// flags in form 04555 - will by parsed in both 8- and 16-based
	void SetChown(const QString & pfn);

	bool IsOwnerRoot(const QString & pfn);
	
	void ReleaseRights();

	const QString & GetCmd();			// both for StartPing()
	QStringList FormatArgs(const QString & adr);
};

#endif // OSSPECIFIC_H
