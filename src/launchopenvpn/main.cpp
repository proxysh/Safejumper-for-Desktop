#include <QFile>
#include <QFileInfo>
#include <QString>


#include "pathhelper.h"
#include "common.h"

int parseArguments(const char * thisprog, const char * filename, QString & openVPNPath, QStringList & args)
{
    printf("@@----- 01\n");
    openVPNPath.clear();
    args.clear();

    QFile pa(filename);
    if (!pa.exists())
        return 101;
    printf("@@----- 02\n");
    if (!pa.open(QIODevice::ReadOnly))
        return 102;
    printf("@@----- 03\n");
    QByteArray ba = pa.readAll();
    if (ba.isEmpty())
        return 103;
    QString params(ba);
    printf("@@----- 04\n");
    args = params.split(' ', QString::SkipEmptyParts);
    printf("@@@@@@@@@@@@\n");

    QFileInfo fi(thisprog);
#ifdef Q_OS_MAC
    openVPNPath = fi.canonicalPath() + PathHelper::Instance()->OvRelativePfn();   // "/openvpn/openvpn-2.3.2/openvpn-executable";
#else
    openVPNPath = PathHelper::Instance()->OpenvpnPathfilename()
            ;
#endif      // Q_OS_MAC
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 2;       // param: tmp file with parameters

    become_root();

    QString openVPNPath;
    QStringList args;
    int r = parseArguments(argv[0], argv[1], openVPNPath, args);
    if (r)
        return r;
    printf("@@ OpenVPN: %s\n", openVPNPath.toStdString().c_str());
    printf("@@ args: %s\n", args.join(' ').toStdString().c_str());

    int r2 = exec_fork(openVPNPath, args);
    printf("@@@ 011\n");
    return r2;
}
