#ifndef PATHHELPER_H
#define PATHHELPER_H

#include <memory>

#include <QString>

class PathHelper
{
public:
    static bool exists();
    static PathHelper * Instance();
    static void cleanup();

    ~PathHelper();

    QString openvpnFilename();
    QString openvpnRelativeFilename();
    QString openvpnLogFilename();
    QString openvpnConfigFilename();
    QString proxyshCaCertFilename();
    QString upScriptFilename();
    QString downScriptFilename();
    QString launchopenvpnFilename();
    QString obfsproxyFilename();
    QString installObfsproxyFilename();
    QString netDownFilename();
    QString safejumperLogFilename(); // "/tmp/Safejumper-debug.log"

    QString resourcesPath();

private:
    QString tempPath(); // Where to keep config file, logs etc.
    PathHelper();
    static std::auto_ptr<PathHelper> _inst;
};

#endif // PATHHELPER_H
