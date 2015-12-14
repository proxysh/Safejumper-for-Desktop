#ifndef SETTING_H
#define SETTING_H

#include <memory>
#include <vector>
#include <QString>

#include "common.h"

// Controller: settings
// major controler in this Singleton while serialized storage in QSettings
// and current storage directly in checkboxes/controls around
// TODO: -2 move all settings controller code to here
class Setting
{
public:
	~Setting();
	static Setting * Instance();
	static void Cleanup() { if (_inst.get() != NULL) delete _inst.release();}
	static bool IsExists() { return (_inst.get() != NULL); }

	bool IsShowNodes();
	bool IsDisableIPv6();
	void ToggleShowNodes(bool v);

	bool IsAutoconnect();
	bool IsStartup();
	bool IsReconnect();
	bool IsInsecureWifi();
	bool IsBlockOnDisconnect();
	bool IsFixDns();

	void SaveServer(int ixsrv, const QString & newsrv);
	void LoadServer();

	void SaveProt(int ix);  // -1 for not selected
	int LoadProt();

	QString Server();			// "" if none
	QString Port();				// "" if none
	QString LocalPort();
	QString Protocol();			// "tcp" : "udp"

	QString Dns1();				// empty string if none / not valid
	QString Dns2();				//

	void SetDefaultDns(const QString & dns1, const QString & dns2);
	QString DefaultDns1() { return _default_dns[0]; }
	QString DefaultDns2() { return _default_dns[1]; }

	static const std::vector<QString> & GetAllProt();
	static const QString & ProtoStr(int ix);

	UVec ForwardPorts();		// load from the GUI; parse errors skipped - GUI should handle errors; empty vector if none

	bool IsCheckForUpdates();
	void UpdateMsgShown();

	void SwitchToNextPort();
	void SwitchToNextNode();

private:
	Setting();
	static std::auto_ptr<Setting> _inst;
	static std::vector<QString> _protocols;

	QString _default_dns[2];
};

#endif // SETTING_H
