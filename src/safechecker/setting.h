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
    static void Cleanup()
    {
        if (_inst.get() != NULL) delete _inst.release();
    }
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }

    bool IsShowNodes();
    bool IsDisableIPv6();
    void ToggleShowNodes(bool v);

    bool IsAutoconnect();
    bool IsStartup();
    bool IsReconnect();
    bool IsInsecureWifi();
    bool IsBlockOnDisconnect();
    bool IsFixDns();
    bool testing();
    void setTesting(bool value);

    void SaveServer(int ixsrv, const QString & newsrv);
    void LoadServer();

    void SaveProt(int ix);  // -1 for not selected
    int LoadProt();
    int CurrProto();			// -1 if none selected

    QString Server();			// "" if none
    int ServerID();				// -1 if none selected
    QString Port();				// "" if none
    QString LocalPort();
    QString Protocol();			// "tcp" : "udp"

    QString Dns1();				// empty string if none / not valid
    QString Dns2();				//

    void SetDefaultDns(const QString & dns1, const QString & dns2);
    QString DefaultDns1()
    {
        return _default_dns[0];
    }
    QString DefaultDns2()
    {
        return _default_dns[1];
    }

    static const std::vector<QString> & GetAllProt();
    static const std::vector<int> & GetAllPorts();
    static const QString & ProtoStr(int ix);
    const QString & CurrProtoStr();

    UVec ForwardPorts();		// load from the GUI; parse errors skipped - GUI should handle errors; empty vector if none

    bool IsCheckForUpdates();
    void UpdateMsgShown();

    void SwitchToNextPort();
    void SwitchToNextNode();

#ifdef MONITOR_TOOL
    void InitLoop();
    // enumerate all the ports and then switch to next node
    // return false after one full cycle
    bool SwitchToNext();
#endif	// MONITOR_TOOL

    static int Encryption();

    static const char * EncText(size_t enc);

private:
    Setting();
    static std::auto_ptr<Setting> _inst;
    static std::vector<QString> _protocols[ENCRYPTION_COUNT];
    static std::vector<int> _ports[ENCRYPTION_COUNT];

    QString _default_dns[2];
    static void PopulateColls(std::vector<QString> & v_strs, std::vector<int> & v_ports, size_t sz, const char ** protocols, const int * ports);

    QString ProtocolSettingsName();
    QString ProtocolSettingsStrName();
    QString EncryptionIx();
    QString LocationSettingsName();
    QString LocationSettingsStrName();
#ifdef MONITOR_TOOL
    int _ixStartPort;
    int _idStartNode;
#endif	// MONITOR_TOOL
    int DetermineNextPort();
    bool mTesting;
};

#endif // SETTING_H
