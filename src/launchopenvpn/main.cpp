#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>


#include "pathhelper.h"
#include "common.h"

int parseArguments(const char * thisprog, const char * filename, QString & openVPNPath, QStringList & args)
{
    printf("@@----- 01\n");
    openVPNPath = PathHelper::Instance()->openvpnFilename();

    args.clear();

    QFile argumentsFile(filename);
    if (!argumentsFile.exists())
        return 101;
    printf("@@----- 02\n");
    if (!argumentsFile.open(QIODevice::ReadOnly))
        return 102;
    printf("@@----- 03\n");
    QByteArray data = argumentsFile.readAll();
    if (data.isEmpty())
        return 103;
    QString params(data);
    printf("@@----- 04\n");
    args = params.split(' ', QString::SkipEmptyParts);
    printf("@@@@@@@@@@@@\n");

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 2;       // param: tmp file with parameters

    become_root();

    QCoreApplication app(argc, argv);

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
