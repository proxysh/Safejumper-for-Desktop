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
#include <QFile>
#include <QString>
#include <QTranslator>

#include "common.h"

// Controller: settings
// major controler in this Singleton while serialized storage in QSettings
// and current storage directly in checkboxes/controls around
// TODO: -2 move all settings controller code to here
class Setting: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool startup READ startup WRITE setStartup NOTIFY startupChanged)
    Q_PROPERTY(bool autoconnect READ autoconnect WRITE setAutoconnect NOTIFY autoconnectChanged)
    Q_PROPERTY(bool reconnect READ reconnect WRITE setReconnect NOTIFY reconnectChanged)
    Q_PROPERTY(bool disableIPv6 READ disableIPv6 WRITE setDisableIPv6 NOTIFY disableIPv6Changed)
    Q_PROPERTY(bool killSwitch READ blockOnDisconnect WRITE setBlockOnDisconnect NOTIFY killSwitchChanged)
    Q_PROPERTY(bool fixdns READ fixDns WRITE setFixDns NOTIFY fixDnsChanged)
    Q_PROPERTY(bool logging READ logging WRITE setLogging NOTIFY loggingChanged)
    Q_PROPERTY(bool notifications READ notifications WRITE setNotifications NOTIFY notificationsChanged)
    Q_PROPERTY(bool pingEnabled READ pingEnabled WRITE setPingEnabled NOTIFY pingEnabledChanged)
    Q_PROPERTY(QString dns1 READ dns1 WRITE setDNS1 NOTIFY dns1Changed)
    Q_PROPERTY(QString dns2 READ dns2 WRITE setDNS2 NOTIFY dns2Changed)
    Q_PROPERTY(QString localPort READ localPort WRITE setLocalPort NOTIFY localPortChanged)
    Q_PROPERTY(bool rememberMe READ rememberMe WRITE setRememberMe NOTIFY rememberMeChanged)

    Q_PROPERTY(QString login READ login WRITE setLogin NOTIFY loginChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString version READ version)

    Q_PROPERTY(int encryption READ encryption WRITE setEncryption NOTIFY encryptionChanged)
    Q_PROPERTY(QString encryptionName READ encryptionName NOTIFY encryptionChanged)
    Q_PROPERTY(QString encryptionCount READ encryptionCount)

    Q_PROPERTY(int defaultPortIndex READ defaultPortIndex WRITE setDefaultPort NOTIFY protocolChanged)
    Q_PROPERTY(QString defaultPort READ defaultPort NOTIFY protocolChanged)
    Q_PROPERTY(QString portCount READ portCount NOTIFY encryptionChanged)

    Q_PROPERTY(QStringList ports READ currentEncryptionPorts NOTIFY encryptionChanged)

    Q_PROPERTY(int language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY languageChanged)
    Q_PROPERTY(QStringList languages READ languages NOTIFY languageChanged)

    Q_PROPERTY(int server READ serverID WRITE setServer NOTIFY serverChanged)
    // Property for which server is visible in the "favorite" box above the server
    // card on the map page
    Q_PROPERTY(int favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)

public:
    ~Setting();
    static Setting *instance();
    static void cleanup();
    static bool exists();

    bool startup();
    void setStartup(bool v);

    bool autoconnect();
    void setAutoconnect(bool v);

    bool reconnect();
    void setReconnect(bool v);

    bool disableIPv6();
    void setDisableIPv6(bool v);

    bool blockOnDisconnect();
    void setBlockOnDisconnect(bool v);

    bool fixDns();
    void setFixDns(bool v);

    bool notifications();
    void setNotifications(bool v);

    bool pingEnabled();
    void setPingEnabled(bool v);

    bool showNodes();
    void setShowNodes(bool v);

    bool detectInsecureWifi();
    void setDetectInsecureWifi(bool v);

    bool testing();
    void setTesting(bool value);

    bool logging();
    void setLogging(bool value);

    void setServer(int ixsrv);
    void loadServer();

    void setDefaultPort(int ix);  // -1 for not selected
    void loadProtocol();
    int currentProtocol();			// -1 if none selected

    QString serverAddress();			// "" if none
    int serverID();				// -1 if none selected
    QString port();				// "" if none

    int favorite();
    void setFavorite(int id);

    QString localPort();
    void setLocalPort(QString port);

    bool rememberMe();
    void setRememberMe(bool v);

    QString login();
    void setLogin(QString login);

    QString password();
    void setPassword(QString password);

    QString tcpOrUdp();			// "tcp" : "udp"

    QString dns1();				// empty string if none / not valid
    void setDNS1(QString value);
    QString dns2();				//
    void setDNS2(QString value);

    void setDefaultDNS(const QString & dns1, const QString & dns2);
    QString defaultDNS1();
    QString defaultDNS2();

    const QString & protocolName(int ix);
    const QString & currentProtocolName();

    const QString forwardPortsString();
    UVec forwardPorts();		// load from the GUI; parse errors skipped - GUI should handle errors; empty vector if none
    void setForwardPorts(QString portsString);

    bool checkForUpdates();
    void updateMessageShown();

    void switchToNextPort();
    void switchToNextNode();

    int encryption();
    void setEncryption(int enc);

    // Name of current encryption type
    QString encryptionName();

    Q_INVOKABLE QString encryptionNameForIndex(int index);

    Q_INVOKABLE QString protocolNameForIndex(int encryption, int index);

    // A string to show how many encryption options are available
    QString encryptionCount();

    // A string to show how many port options are available
    QString portCount();

    QString defaultPort();
    int defaultPortIndex();

    QString version();

    // Set ports available for the given encryption type
    void setEncryptionPorts(int encryptionType, QStringList ports, QList<int> portNumbers);
    QStringList currentEncryptionPorts();

    Q_INVOKABLE QStringList portsForEncryption(int encryptionType) const;

    int language();
    void setLanguage(int language);
    QString currentLanguage();

    QStringList languages();

    const QStringList favorites() const;
    void addFavorite(const QString &url);
    void removeFavorite(const QString &url);

    // Get server's encryption setting by it's hostname if stored.
    // If this server has no encryption setting, just give the current default encryption
    Q_INVOKABLE int serverEncryption(const QString &serverAddress);
    Q_INVOKABLE void setServerEncryption(const QString &serverAddress, int encryption);

    // Get server's protocol for given encryption type if any, otherwise get default protocol
    Q_INVOKABLE int serverProtocol(const QString &serverAddress, int encryption);
    // Set server's protocol for given encryption type
    Q_INVOKABLE void setServerProtocol(const QString &serverAddress, int encryption, int protocol);

    QString portNumber(int encryptionType, int protocol);
    QString tcpOrUdp(int encryptionType, int protocol);

signals:
    void showNodesChanged();
    void detectInsecureWifiChanged();
    void encryptionChanged();
    void protocolChanged();
    void serverChanged();
    void favoriteChanged();

    void startupChanged();
    void autoconnectChanged();
    void reconnectChanged();
    void disableIPv6Changed();
    void killSwitchChanged();
    void fixDnsChanged();
    void dns1Changed();
    void dns2Changed();
    void loggingChanged();
    void localPortChanged();
    void notificationsChanged();
    void pingEnabledChanged();
    void rememberMeChanged();
    void loginChanged();
    void passwordChanged();
    void languageChanged();

private:
    Setting();
    static std::auto_ptr<Setting> mInstance;

    QString mDefaultDNS[2];
    bool mTesting;

    QMap<int, QStringList> mPortsByEncryption;
    QMap<int, QList<int> > mPortumbersByEncryption;

    static void PopulateColls(std::vector<QString> & v_strs, std::vector<int> & v_ports, size_t sz, const char ** protocols, const int * ports);

    QString ProtocolSettingsName();
    QString ProtocolSettingsStrName();
    QString EncryptionIx();
    QString LocationSettingsName();
    QString LocationSettingsStrName();
    int determineNextPort();
#ifdef Q_OS_LINUX
    void delete_startup(QFile &f);
#endif
    QSettings mSettings;

    QTranslator mTranslator;
};

#endif // SETTING_H
