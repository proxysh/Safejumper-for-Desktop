#ifndef COMMON_H
#define COMMON_H

#include <QDialog>
#include <QString>
#include <QSettings>
#include <QApplication>
#include <QNetworkRequest>

#include "stringize.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef __GNUC__
#include <ext/hash_map>

namespace std
{
 using namespace __gnu_cxx;
}

#else
#include <unordered_map>
#endif

#define ENCRYPTION_RSA 0
#define ENCRYPTION_OBFS_TOR 1
#define ENCRYPTION_ECC 2
#define ENCRYPTION_ECCXOR 3
#define ENCRYPTION_COUNT 4

#define PORT_FORWARD_MAX 5

extern QApplication * g_pTheApp;

bool IsValidIp(const QString & ip);
bool IsValidPort(const QString & s);

bool OpenUrl(const char * url);
bool OpenUrl_Support();
bool OpenUrl_Panel();
bool OpenUrl_Earn();
bool OpenUrl_Bug();
bool OpenUrl_Update();

//extern const QString & GetSettingsFn();

#define SETTINGS_OBJ QSettings settings("proxy.sh", "Safejumper")
//#define CB_SAVE_VALUE(name) do { SETTINGS_OBJ; settings.setValue(BOOST_PP_STRINGIZE(name), ui-> ##name## ->isChecked()); } while (0)

void SaveCb(const char * name, bool val);

struct AServer
{
	QString address;	// IP or DNs
	QString name;		// "Chile Hub" - Hub at the end indicates hub
	QString load;			// double
};

#ifdef std::unordered_map
#define THE_HM std::unordered_map
#else
#ifdef std::hash_map
#define THE_HM std::hash_map
#else
#ifdef __gnu_cxx::hash_map
#define THE_HM __gnu_cxx::hash_map
#else
#ifdef std::tr1::hash_map
#define THE_HM std::tr1::hash_map
#else
#ifdef stdext::hash_map
#define THE_HM stdext:hash_map
#else
#define THE_HM std::map
#endif
#endif
#endif
#endif
#endif

typedef THE_HM<QString, size_t>  HMSI;

typedef std::map<std::string, int> SIMap;
typedef std::map<int, int> IIMap;

typedef std::vector<unsigned int> UVec;
typedef std::set<unsigned int> USet;

// escape \ and "
QString EscapePsw(const QString & raw);

QNetworkRequest BuildRequest(const QUrl & u);

#endif // COMMON_H
