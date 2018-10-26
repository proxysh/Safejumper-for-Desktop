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

#include "application.h"
#include "authmanager.h"
#include "common.h"
#include "log.h"
#include "osspecific.h"

#include <QApplication>
#include <QDir>

//In future, we’ll add things such as “OpenVPN with XOR TCP 448” or “OpenVPN with TOR UDP 4044”.

static const QString kLoggingKey = "logging";
static const QString kNotificationsKey = "notifications";
static const QString kEncryptionKey = "encryption";
static const QString kLanguageKey = "language";

std::vector<QString> Setting::mProtocols[ENCRYPTION_COUNT];
std::vector<int> Setting::mPorts[ENCRYPTION_COUNT];

Setting::Setting()
    :mTesting(false)
{
    mDefaultDNS[0] = "146.185.134.104";
    mDefaultDNS[1] = "192.241.172.159";

    Log::instance()->enableLogging(logging());

    setLanguage(language());
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

//const std::vector<QString> & Setting::currentEncryptionProtocols()
//{
//    int enc = encryption();
//    if (mProtocols[enc].empty()) {
//        switch (enc) {
//        case ENCRYPTION_TLSCRYPT:
//        case ENCRYPTION_RSA: {
//            const char * gs_protocols [] = {
//                "TCP 80 (RSA)"
//                , "TCP 110 (RSA)"
//                , "TCP 443 (RSA)"

//                , "TCP 843 (RSA)"

//                , "UDP 53 (RSA)"

//                , "UDP 1194 (RSA)"
//                , "UDP 1443 (RSA)"
//                , "UDP 8080 (RSA)"
//                , "UDP 9201 (RSA)"
//            };
//            const int gs_ports [] = {
//                80
//                , 110
//                , 443
//                , 843
//                , 53
//                , 1194
//                , 1443
//                , 8080
//                , 9201
//            };

//            size_t sz = sizeof(gs_protocols)/sizeof(gs_protocols[0]);
//            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols, gs_ports);
//            break;
//        }

//        case ENCRYPTION_TOR_OBFS2: {
//            const char * gs_protocols [] = {
//                "TCP 888 (RSA+TOR)"
//            };
//            const int gs_ports [] = {
//                888
//            };
//            size_t sz = sizeof(gs_protocols)/sizeof(gs_protocols[0]);
//            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols, gs_ports);
//            break;
//        }
//        case ENCRYPTION_TOR_OBFS3: {
//            const char *gs_protocols [] = {
//                "TCP 898 (RSA+TOR)"
//            };
//            const int gs_ports [] = {
//                898
//            };
//            size_t sz = sizeof(gs_protocols)/sizeof(gs_protocols[0]);
//            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols, gs_ports);
//            break;
//        }
//        case ENCRYPTION_TOR_SCRAMBLESUIT: {
//            const char *gs_protocols [] = {
//                "TCP 988 (RSA+TOR)"
//            };
//            const int gs_ports [] = {
//                988
//            };
//            size_t sz = sizeof(gs_protocols)/sizeof(gs_protocols[0]);
//            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols, gs_ports);
//            break;
//        }
//        case ENCRYPTION_ECC: {
//            const char * gs_protocols [] = {
//                "TCP 465 (ECC)"
//            };
//            const int gs_ports [] = {
//                465
//            };
//            size_t sz = sizeof(gs_protocols)/sizeof(gs_protocols[0]);
//            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols, gs_ports);
//            break;
//        }
//        case ENCRYPTION_ECCXOR: {
//            const char * gs_protocols [] = {
//                "TCP 995 (ECC+XOR)"
//            };
//            const int gs_ports [] = {
//                995
//            };
//            size_t sz = sizeof(gs_protocols)/sizeof(gs_protocols[0]);
//            PopulateColls(mProtocols[enc], mPorts[enc], sz, gs_protocols, gs_ports);
//            break;
//        }
//        default:
//            throw std::runtime_error("invalid encryption index");
//        }
//    }
//    return mProtocols[enc];
//}

//const std::vector<int> & Setting::currentEncryptionPorts()
//{
//    const std::vector<QString> & pr = currentEncryptionProtocols();		// force init
//    if (!pr.empty()) {
//        int enc = encryption();
//        return mPorts[enc];
//    } else {
//        return mPorts[0];
//    }
//}

std::auto_ptr<Setting> Setting::mInstance;
Setting * Setting::instance()
{
    if (!mInstance.get())
        mInstance.reset(new Setting());
    return mInstance.get();
}

void Setting::cleanup()
{
    if (mInstance.get() != nullptr)
        delete mInstance.release();
}

bool Setting::exists()
{
    return (mInstance.get() != nullptr);
}

bool Setting::showNodes()
{
    return mSettings.value("cb_ShowNodes", false).toBool();
}

void Setting::setShowNodes(bool v)
{
    mSettings.setValue("cb_ShowNodes", v);
    emit showNodesChanged();
}

bool Setting::disableIPv6()
{
    return mSettings.value("cb_DisableIpv6", true).toBool();
}

void Setting::setDisableIPv6(bool v)
{
    mSettings.setValue("cb_DisableIpv6", v);
    emit disableIPv6Changed();
}

bool Setting::autoconnect()
{
    return mSettings.value("cb_AutoConnect", false).toBool();
}

void Setting::setAutoconnect(bool v)
{
    mSettings.setValue("cb_AutoConnect", v);
    emit autoconnectChanged();
}

bool Setting::detectInsecureWifi()
{
    return mSettings.value("cb_InsecureWiFi", false).toBool();
}

void Setting::setDetectInsecureWifi(bool v)
{
    mSettings.setValue("cb_InsecureWiFi", v);
    emit detectInsecureWifiChanged(); // Send signal so login window can start wifi watcher timer
}

bool Setting::blockOnDisconnect()
{
    return mSettings.value("cb_BlockOnDisconnect", false).toBool();
}

void Setting::setBlockOnDisconnect(bool v)
{
    mSettings.setValue("cb_BlockOnDisconnect", v);
    emit killSwitchChanged();
}

bool Setting::fixDns()
{
    return mSettings.value("cb_FixDnsLeak", true).toBool();
}

void Setting::setFixDns(bool v)
{
    mSettings.setValue("cb_FixDnsLeak", v);
    emit fixDnsChanged();
}

bool Setting::notifications()
{
    return mSettings.value(kNotificationsKey, true).toBool();
}

void Setting::setNotifications(bool v)
{
    mSettings.setValue(kNotificationsKey, v);
    emit notificationsChanged();
}

bool Setting::testing()
{
    return mTesting;
}

void Setting::setTesting(bool value)
{
    mTesting = value;
}

bool Setting::logging()
{
    return mSettings.value(kLoggingKey, true).toBool();
}

void Setting::setLogging(bool value)
{
    mSettings.setValue(kLoggingKey, value);
    Log::instance()->enableLogging(value);
    emit loggingChanged();
}

bool Setting::startup()
{
    return mSettings.value("cb_Startup", true).toBool();
}

#ifdef Q_OS_LINUX
static const QString gs_desktop =
    "[Desktop Entry]\n"
    "Type=Application\n"
    "Name=Safejumper\n"
    "Exec=/opt/safejumper/safejumper.sh\n"
    "Icon=/usr/share/icons/hicolor/64x64/apps/safejumper.png\n"
    "Comment=OpenVPN client for proxy.sh\n"
    "X-GNOME-Autostart-enabled=true\n";
#endif

void Setting::setStartup(bool v)
{
    qDebug() << "setStartup called with value " << v;
    mSettings.setValue("cb_Startup", v);
    emit startupChanged();

    // Do the OsSpecific stuff to make it happen
#ifdef Q_OS_WIN
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    static const char * keyname = kOrgName.toStdString().c_str();
    if (v) {
        QString val = "\"" + QCoreApplication::applicationFilePath() + "\"";
        val.replace("/","\\");
        settings.setValue(keyname, val);
    } else {
        settings.remove(keyname);
    }
#endif	// Q_OS_WIN

#ifdef Q_OS_LINUX
    QString dir = QDir::homePath() + "/.config/autostart";
    QString pfn = dir + "/.desktop";
    if (v) {
        QDir d;
        if (d.mkpath(dir)) {
            QFile f(pfn);
            if (f.exists()) {
                f.open(QIODevice::ReadWrite);
                delete_startup(f);
            } else {
                f.open(QIODevice::Append);
            }
            f.write(gs_desktop.toLatin1());
            f.flush();
            f.close();
        }
    } else {
        QFile f(pfn);
        if (f.exists()) {
            f.open(QIODevice::ReadWrite);
            delete_startup(f);
            f.flush();
            f.close();
        }
    }
#endif	// Q_OS_LINUX

#ifdef Q_OS_OSX
    QString dir = QDir::homePath() + "/Library/LaunchAgents";
    QString pfn = dir + QString("/%1.%2.plist").arg(kOrgName).arg(kLowerAppName);
    QFile pa(pfn);
    if (pa.exists()) {
        if (!pa.remove())
            Log::logt("Cannot delete startup file '"+ pfn + "'");
    }
    if (v) {
        QDir d(dir);
        if (!d.exists()) {
            d.mkpath(dir);
        }
        if (!pa.open(QIODevice::WriteOnly)) {
            Log::logt("Cannot open startup file '"+ pfn + "' for writing");
        } else {
            QString s =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
                "<plist version=\"1.0\">\n"
                "<dict>\n"
                "<key>Label</key>\n" +
                QString("<string>%1.%2</string>\n").arg(kOrgName).arg(kLowerAppName) +

                "<key>LimitLoadToSessionType</key>\n"
                "<string>Aqua</string>\n"
                "<key>OnDemand</key>\n"
                "<true/>\n"

                "<key>Program</key>\n"
                "<string>"
                ;

            s +=
                QApplication::applicationFilePath() + "</string>\n"

                //	  "<key>ProgramArguments</key>\n"
                //	  "<array>\n"
                //	  "<string>";
                // <string>arguments_here</string>\n
                //	"</array>\n"

                "<key>KeepAlive</key>\n"
                "<false/>\n"
                "<key>RunAtLoad</key>\n"
                "<true/>\n"
                "</dict>\n"
                "</plist>\n"
                ;
            pa.write(s.toLatin1());
        }
    }
#endif	// Q_OS_OSX
}

bool Setting::reconnect()
{
    return mSettings.value("cb_Reconnect", true).toBool();
}

void Setting::setReconnect(bool v)
{
    mSettings.setValue("cb_Reconnect", v);
    emit reconnectChanged();
}

int Setting::encryption()
{
    int enc = mSettings.value(kEncryptionKey, 0).toInt();
    if (enc < 0 || enc >= ENCRYPTION_COUNT)
        enc = 0;
    return enc;
}

void Setting::setEncryption(int enc)
{
    qDebug() << "setting encryption to " << enc;
    if (enc > -1 && enc < ENCRYPTION_COUNT && enc != mSettings.value(kEncryptionKey, 0).toInt()) {
        mSettings.setValue(kEncryptionKey, enc);

        loadProtocol(); // When encryption changes, we need to load protocols
        emit encryptionChanged();
        emit protocolChanged();
    }
}

QString Setting::encryptionName()
{
    int currentEncryption = encryption();
    if (currentEncryption >= ENCRYPTION_COUNT ||
            currentEncryption >= encryptionNames.size())
        currentEncryption = ENCRYPTION_TLSCRYPT;
    return encryptionNames.at(currentEncryption);
}

QString Setting::encryptionCount()
{
    return tr("%1 OPTIONS AVAILABLE").arg(encryptionNames.count());
}

QString Setting::portCount()
{
    return tr("%1 OPTIONS AVAILABLE").arg(currentEncryptionPorts().size());
}

QString Setting::defaultPort()
{
    return currentProtocolName();
}

int Setting::defaultPortIndex()
{
    return currentProtocol();
}

QString Setting::version()
{
    return APPLICATION_VERSION;
}

void Setting::setEncryptionPorts(int encryptionType, QStringList ports, QList<int> portNumbers)
{
    mPortsByEncryption.insert(encryptionType, ports);
    mPortumbersByEncryption.insert(encryptionType, portNumbers);

    qDebug() << "Setting ports for encryption " << encryptionType
             << " to " << ports.join(", ");
    if (encryptionType == encryption()) {
        // emit that the encryption type changed so port list will be reloaded in the ui
        emit encryptionChanged();
        emit protocolChanged();
    }
}

QStringList Setting::currentEncryptionPorts()
{
    int enc = encryption();
    return mPortsByEncryption.value(enc);
}

int Setting::language()
{
    return mSettings.value(kLanguageKey, languageEnglish).toInt();
}

void Setting::setLanguage(int language)
{
    if (language >= FIRST_LANGUAGE && language <= LAST_LANGUAGE) {
        Log::logt(QString("Changing language to %1").arg(kLanguageNames.at(language)));
        mSettings.setValue(kLanguageKey, language);
        // Switch gui translation
        g_pTheApp->removeTranslator(&mTranslator);
        mTranslator.load(kLanguageTranslations.at(language));
        g_pTheApp->installTranslator(&mTranslator);

        kLanguageNames.clear();
        kLanguageNames << QObject::tr("English");
        kLanguageNames << QObject::tr("Simplified Chinese");

        emit languageChanged();
    }
}

QString Setting::currentLanguage()
{
    return kLanguageNames.at(language());
}

QStringList Setting::languages()
{
    return kLanguageNames;
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
void Setting::setDefaultPort(int ix)
{
    if (_loading_protocol)
        return;
    mSettings.setValue(ProtocolSettingsName(), ix);
    QString s;
//    if (ix > -1 && ix < (int)currentEncryptionProtocols().size())
//        s = currentEncryptionProtocols().at(ix);
    mSettings.setValue(ProtocolSettingsStrName(), s);
    emit protocolChanged();
}

void Setting::loadProtocol()
{
    _loading_protocol = true;
//    int ix = mSettings.value(ProtocolSettingsName(), -1).toInt();
//    if (ix > -1) {
//        if (ix >= (int)currentEncryptionProtocols().size())
//            ix = -1;
//        else {
//            QString s = mSettings.value(ProtocolSettingsStrName(), "").toString();
//            if (s != currentEncryptionProtocols().at(ix))
//                ix = -1;
//        }
//    }
//    if (ix < 0) {
//        ix = rand() % currentEncryptionProtocols().size();
//    }
//    setProtocol(ix);

    _loading_protocol = false;
}

static QString gs_Empty = "";
const QString & Setting::protocolName(int ix)
{
    if (ix > -1 && ix < currentEncryptionPorts().size())
        return currentEncryptionPorts().at(ix);
    else
        return gs_Empty;
}

const QString & Setting::currentProtocolName()
{
    return protocolName(currentProtocol());
}

const QString Setting::forwardPortsString()
{
    return mSettings.value("e_Ports", "").toString();
}

int Setting::currentProtocol()
{
    return mSettings.value(ProtocolSettingsName(), 0).toInt();
}

void Setting::setServer(int ixsrv)
{
    AServer *se = AuthManager::instance()->getServer(ixsrv);
    QString newsrv = se->name();
    mSettings.setValue(LocationSettingsName(), ixsrv);
    mSettings.setValue(LocationSettingsStrName(), newsrv);
    emit serverChanged();
}

void Setting::loadServer()
{
    Log::logt("Setting loadServer called");
    if (AuthManager::instance()->currentEncryptionServers().empty()) {
        Log::logt("Returning because we don't have a server list");
        return;		// cannot select in empty list
    }

    int savedsrv = mSettings.value(LocationSettingsName(), -1).toInt();
    QString savedname = mSettings.value(LocationSettingsStrName(), "undefined").toString();
    Log::logt("In Setting::loadServer previously saved server name is " + savedname);
    Log::logt("Previously saved server index is " + QString::number(savedsrv));

    int ixsrv = -1;
    // verify that the sever exists
    if (savedsrv > -1) {
        AServer  *sr1 = AuthManager::instance()->getServer(savedsrv);
        if (!sr1->name().isEmpty()) {
            if (sr1->name() == savedname)
                ixsrv = savedsrv;
            else if (savedname != "undefined") {
                ixsrv = AuthManager::instance()->serverIxFromName(savedname);
                Log::logt("Server names didn't match their indexes, server index is " + QString::number(ixsrv));
            }
        } else {
            if (savedname != "undefined") {
                ixsrv = AuthManager::instance()->serverIxFromName(savedname);
                Log::logt("Server names didn't match their indexes, server index is " + QString::number(ixsrv));
            }
        }
    }

    if (ixsrv < 0)
        ixsrv = AuthManager::instance()->getServerToJump();

    Log::logt("Setting::loadServer server index to set is " + QString::number(ixsrv));
    // initiate population of the Location drop-down;
    // will call Setting::IsShowNodes() which will initiate scr_settings and load checkboxes
    setServer(ixsrv);
}

QString Setting::serverAddress()
{
    QString s;
    int ix = serverID();
    if (ix > -1)
        s = AuthManager::instance()->getServer(ix)->address();
    return s;
}

int Setting::serverID()
{
    return mSettings.value(LocationSettingsName()).toInt();
}

QString Setting::port()
{
    int ix = currentProtocol();
    int p = 80;
    int enc = encryption();
    std::vector<int> & v_ports = mPorts[enc];
    if (ix > -1 && ix < (int)v_ports.size())
        p = v_ports.at(ix);
    return QString::number(p);
}

int Setting::determineNextPort()
{
    int ix = currentProtocol();
    ++ix;
    int enc = encryption();
    std::vector<int> & v_ports = mPorts[enc];
    if (ix >= (int)v_ports.size())
        ix = 0;
    return ix;
}

void Setting::switchToNextPort()
{
    int ix = determineNextPort();
    setDefaultPort(ix);
}

void Setting::switchToNextNode()
{
    // Get the current encryption server ids list
    QList<int> ids = AuthManager::instance()->currentEncryptionServers();
    // Find the current server
    int ix = serverID();
    // Go to the next one
    int which = ids.indexOf(ix);
    if (which != -1 && which + 1 < ids.size())
        setServer(ids.at(which + 1));
}

QString Setting::localPort()
{
    QString p = mSettings.value("e_LocalPort", "9090").toString();
    if (p.isEmpty())
        p = "6842";
    return p;
}

void Setting::setLocalPort(QString port)
{
    if (IsValidPort(port)) {
        mSettings.setValue("e_LocalPort", port);
        emit localPortChanged();
    }
}

bool Setting::rememberMe()
{
    return mSettings.value("cb_Rememberme", true).toBool();
}

void Setting::setRememberMe(bool v)
{
    mSettings.setValue("cb_Rememberme", v);
    emit rememberMeChanged();
}

QString Setting::login()
{
    return mSettings.value("eLogin").toString();
}

void Setting::setLogin(QString login)
{
    mSettings.setValue("eLogin", login);
    emit loginChanged();
}

QString Setting::password()
{
    return mSettings.value("ePsw").toString();
}

void Setting::setPassword(QString password)
{
    mSettings.setValue("ePsw", password);
    emit passwordChanged();
}

QString Setting::tcpOrUdp()
{
    QString description = protocolName(currentProtocol());
    if (description.contains("udp", Qt::CaseInsensitive))
        return "udp";
    else
        return "tcp";
}

QString Setting::dns1()
{
    return mSettings.value("e_PrimaryDns", defaultDNS1()).toString();
}

void Setting::setDNS1(QString value)
{
    mSettings.setValue("e_PrimaryDns", value);
    emit dns1Changed();
}

QString Setting::dns2()
{
    return mSettings.value("e_SecondaryDns", defaultDNS2()).toString();
}

void Setting::setDNS2(QString value)
{
    mSettings.setValue("e_SecondaryDns", value);
    emit dns2Changed();
}

UVec Setting::forwardPorts()
{
    QString portsString = forwardPortsString();
    bool valid = true;
    UVec v;
    QStringList pp = portsString.split(",");
    for (int k = 0; k < pp.length() && valid; ++k) {
        if (!IsValidPort(pp[k])) {
            valid = false;
            v.clear();
        } else {
            v.push_back(pp[k].toInt());
        }
    }

    std::sort(v.begin(), v.end());
    return v;
}

void Setting::setForwardPorts(QString portsString)
{
    mSettings.setValue("e_Ports", portsString);
}

static const char * gs_upd_name = "LastUpdMsg";
static const char * gs_undefined = "undefined";
bool Setting::checkForUpdates()
{
    bool is = true;
    QString saved = mSettings.value(gs_upd_name, gs_undefined).toString();
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
    uint t = QDateTime::currentDateTimeUtc().toTime_t();
    mSettings.setValue(gs_upd_name, QString::number(t));
}

#ifdef Q_OS_LINUX
// f - opened for read-write
void Setting::delete_startup(QFile & f)
{

    int ncount = gs_desktop.count('\n');
    int n3 = 0;
    for (int k = 0; k < 3; ++k)
        n3 = gs_desktop.indexOf('\n', n3 + 1);
    if (n3 < 0)
        return;

    QString s(f.readAll());

    QString begining = gs_desktop.mid(0, n3);
    int p = s.indexOf(begining);
    if (p < 0)
        return;

    int lastn = p;
    for (int k = 0; lastn > -1 && k < ncount; ++k)
        lastn = gs_desktop.indexOf('\n', lastn + 1);
    if (lastn < 0) {
        Log::logt("Openned .desktop file but cannot find proper " + QString::number(ncount) + " lines");
        return;	// err
    }

    QString remains = s.mid(0, p);
    remains += s.mid(lastn + 1);

    QByteArray out = remains.toLatin1();
    f.resize(out.length());
    f.write(out);
    f.flush();
}
#endif
