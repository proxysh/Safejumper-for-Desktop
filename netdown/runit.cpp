#include "runit.h"

#include <iostream>

#include <QProcess>

void runit(const char * cmd, unsigned int ms_delay)
{
	QProcess p;

	std::cout << "@@ Running cmd '" << cmd << "'" << std::endl;
	p.start(cmd);
	if (!p.waitForFinished(ms_delay))
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
