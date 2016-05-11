//#include <QCoreApplication>
#include <QProcess>

#include <iostream>
#include <cstdio>
#include "common.h"

template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

void runit(const char * cmd)
{
	QProcess p;

	std::cout << "@@ Running cmd '" << cmd << "'" << std::endl;
	p.start(cmd);
	if (!p.waitForFinished(500))
	{
		QString s1(p.readAllStandardError());
		std::cout << "@@ error: " << s1.toStdString() << std::endl;
		if (QProcess::NotRunning != p.state())
		{
			p.terminate();
			p.kill();
		}
	}
	else
	{
		QString s0(p.readAllStandardOutput());
		if (!s0.isEmpty())
			std::cout << "@@ output: " << s0.toStdString() << std::endl;
	}
}

static const char * cmds [] =
{
#ifdef Q_OS_MAC
	"/sbin/ifconfig en0 down",
	"/sbin/ifconfig en1 down",
	"/usr/sbin/networksetup -setairportpower Wi-Fi off"
#else
	"/sbin/ifconfig eth0 down",
	"/sbin/ifconfig eth1 down",
#endif
};

int main(int argc, char *argv[])
{
printf("@@@ 01\n");
	become_root();
printf("@@@ 02\n");

	for (int k = 0; k < arraysize(cmds); ++k)
		runit(cmds[k]);

printf("@@@ 05\n");
	return 0;

//	QCoreApplication a(argc, argv);
//	return a.exec();
}
