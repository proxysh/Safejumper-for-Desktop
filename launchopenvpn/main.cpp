#include <QApplication>
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
	pfnOV = PathHelper::Instance()->OpenvpnPathfilename();
#endif		// Q_OS_MAC
	//QString pfnov = "/Users/aa/src/qt/build-safejumper-Debug/Safejumper.app/Contents/Resources/openvpn/openvpn-2.3.2/openvpn-executable";
	return 0;
}

int exec_fork(const QString & pfnOV, const QStringList & args)
{
	pid_t processId = 0;
printf("@@@ parent launcher  pid = %d\n", processId);

	if ((processId = fork()) == 0)
	{	// child: start OpenVPN here
printf("@@@ CHILD launcher  pid = %d\n", processId);

		//char app[] = "/bin/echo";
		//char * const argv[] = { app, "success", NULL };

		//char app[] = "/usr/bin/whoami";
		//char * const argv[] = { app, NULL };

//		char app[] = "/Users/aa/src/qt/build-safejumper-Debug/Safejumper.app/Contents/Resources/openvpn/openvpn-2.3.2/openvpn-executable";
//			char * const argv[] = { app, "success", NULL };

		std::string bufapp = pfnOV.toStdString();
		const char * app = bufapp.c_str();

		QVector<QString> v0 = args.toVector();
		std::vector<std::string> buf;
		buf.push_back(app);
		for (int k = 0; k < v0.length(); ++k)
			buf.push_back(v0[k].toStdString());

		std::vector<char *> argvbuf;
		for (size_t k = 0; k < buf.size(); ++k)
		{
			const char * p0 = buf.at(k).c_str();
			char * p1 = const_cast<char*>(p0);
			argvbuf.push_back(p1);
		}
		argvbuf.push_back(NULL);
//char * const argv[] = (char * const [])&argvbuf[0];
printf("@@ cmd = %s\n", app);
//printf("@@ args = %s\n", );
printf("@@ before execv()\n");
//		if (execv(bufapp.c_str(), argvbuf) < 0)

		char * const * argv = &argvbuf[0];
		if (execv(app, argv) < 0)
		//if (execv(app, &argvbuf[0]) < 0)				// never return on success
		{
printf("@@  execv() < 0\n");
			perror("execv error");
printf("@@  execv() < 0\n");
		}
printf("@@ after execv()\n");

	}
	else	// fork()
	{	// parent launcher
printf("@@@ parent launcher  pid = %d\n", processId);
		if (processId < 0)
		{
			perror("@@@@@@ fork() error ###########");
		}
		else
		{
			printf("@@@@@@ fork() parent EXIT_SUCCESS  pid = %d\n", processId);
			return EXIT_SUCCESS;
		}
	}
	perror("EXIT_FAILURE");
	return EXIT_FAILURE;

}

int exec_QProcess(const QString & pfnOV, const QStringList & args)
{
	QApplication::setSetuidAllowed(true);
printf("@@ before QProcess::execute()");
	int r = QProcess::execute(pfnOV, args);
printf("@@ QProcess::execute() returns %d\n", r);

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

printf("@@@@@@@@@@@@3\n");
	return r;
}

//
//
QApplication * g_pTheApp;
int main(int argc, char *argv[])
{
	if (argc < 2)
		return 2;		// param: tmp file with parameters
printf("@@@ 01\n");


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
