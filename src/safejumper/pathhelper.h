#ifndef PATHHELPER_H
#define PATHHELPER_H

#include <memory>

#include <QString>

class PathHelper
{
public:
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }
    static PathHelper * Instance();
    static void Cleanup()
    {
        if (_inst.get() != NULL) delete _inst.release();
    }
    ~PathHelper();

    QString OpenvpnPathfilename();
    QString OvRelativePfn();		//"/openvpn/openvpn-2.3.2/openvpn-executable";

    QString OpenvpnWorkdir();
    QString OpenvpnLogPfn();
    QString OpenvpnConfigPfn();
    QString ProxyshCaCert();
    QString UpScriptPfn();
    QString DownScriptPfn();
    QString LauncherPfn();

    QString ObfsproxyPfn();
    QString ObfsInstallerPfn();

    QString NetDownPfn();

    QString ResourcesPath();			// Resources
    QString ContentPath();

    QString LogPfn();		// "/tmp/Safejumper-debug.log"

private:
    PathHelper();
    static std::auto_ptr<PathHelper> _inst;
    QString ScriptPath();

    QString _openvpn;
};

#endif // PATHHELPER_H
