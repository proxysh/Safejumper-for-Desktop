//#include <QApplication>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QVector>
#include <QString>


#include <memory>

#include <sys/unistd.h>

#include "launcherworker.h"
#include "pathhelper.h"

#include "common.h"

int GetStrs(const char * thisprog, const char * pfn, QString & pfnOV, QStringList & args)
{
printf("@@----- 01\n");
	pfnOV.clear();
	args.clear();

	QString pfn1(pfn);
	QFile pa(pfn1);
	if (!pa.exists()) return 101;
printf("@@----- 02\n");
	if (!pa.open(QIODevice::ReadOnly)) return 102;
printf("@@----- 03\n");
	QByteArray ba = pa.readAll();
	if (ba.isEmpty()) return 103;
	QString params(ba);
printf("@@----- 04\n");
	args = params.split(' ', QString::SkipEmptyParts);
printf("@@@@@@@@@@@@\n");
	//QString pfnov = PathHelper::Instance()->OpenvpnPathfilename();

	QFileInfo fi(thisprog);
#ifdef Q_OS_MAC
	pfnOV = fi.canonicalPath() + PathHelper::Instance()->OvRelativePfn();	// "/openvpn/openvpn-2.3.2/openvpn-executable";
#else
	pfnOV = PathHelper::Instance()->OpenvpnPathfilename() 
//+ ".sh"
	;
#endif		// Q_OS_MAC
	//QString pfnov = "/Users/aa/src/qt/build-safejumper-Debug/Safejumper.app/Contents/Resources/openvpn/openvpn-2.3.2/openvpn-executable";
	return 0;
}



int exec_QProcess(const QString & pfnOV, const QStringList & args)
{
	int ret = 0;


printf("@@ before QProcess::execute(pfnOV, args)\n");
	bool b = QProcess::execute(pfnOV, args);
	printf("@@ QProcess::start() execute %s\n", (b ? "true" : "false"));

//printf("@@ before QProcess::start()");
//	QProcess pr;
//	pr.start(pfnOV, args);
//	bool b = pr.waitForStarted(2000);
//printf("@@ QProcess::start() returns %s\n", (b ? "true" : "false"));
//	if (QProcess::NotRunning == pr.state())
//		ret = pr.exitCode();

// whoami:
//QProcess pr;
//pr.start("/usr/bin/whoami");
//bool b6 = pr.waitForFinished(2000);
//printf("@@ QProcess::start() returns %s\n", (b6 ? "true" : "false"));
//QString s6(pr.readAllStandardOutput());
//printf("@@ stdout = %s\n", s6.toStdString().c_str());
//printf("@@@@@@@@@@@@4\n");

	printf("@@@@@@@@@@@@3\n");

	return ret;
}

#if 0
printf("Exec NULL\n");
	int r5 = system(NULL);
printf("system() returns %d\n", r5);


QString cmd = pfnov + " " + params;
std::string s0 = cmd.toStdString();
printf("Entire command is: %s\n", s0.c_str());
	int r = system(s0.c_str());
printf("system() returns %d\n", r);

#endif


//QApplication * g_pTheApp;
int main(int argc, char *argv[])
{
	if (argc < 2)
		return 2;		// param: tmp file with parameters
printf("@@@ 01\n");

//	QApplication::setSetuidAllowed(true);
//	QApplication a(argc, argv);
//	g_pTheApp = &a;

	become_root();

	QString pfnOV;
	QStringList args;
	int r = GetStrs(argv[0], argv[1], pfnOV, args);
	if (r)
		return r;
printf("@@ OpenVPN: %s\n", pfnOV.toStdString().c_str());
printf("@@ args: %s\n", args.join(' ').toStdString().c_str());

//args.clear();

//args << "-l" << "/";
//pfnOV = "/bin/ls";

//pfnOV = "/usr/bin/whoami";

//pfnOV = "/usr/bin/sudo";
//args << "/bin/ls" << "-l" << "/";

	int r2 = exec_fork(pfnOV, args);
//	int r2 = exec_QProcess(pfnOV, args);
//char ch = getchar(); char t = ch;char t2 = t;ch++;ch = t2;
printf("@@@ 011\n");
	return r2;



#if 0
	//QApplication::setSetuidAllowed(true);
printf("@@@ 02\n");
	QApplication::setSetuidAllowed(true);
	QApplication a(argc, argv);
	g_pTheApp = &a;
printf("@@@ 03\n");
	std::auto_ptr<LauncherWorker> sp(new LauncherWorker(argv[1], &a));
	QTimer::singleShot(10, sp.get(), SLOT(Timer_Constructed()));
printf("@@@ 04\n");
	return a.exec();		// main event loop
printf("@@@ 05\n");
#endif

}
