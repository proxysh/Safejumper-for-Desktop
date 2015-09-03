#include "launcherworker.h"
#include <QFile>
#include <QApplication>
#include <QProcess>

#include "pathhelper.h"

extern QApplication * g_pTheApp;

LauncherWorker::LauncherWorker(const QString & pfnParams, QObject *parent)
	: QObject(parent)
	, _pfnParams(pfnParams)
{}

LauncherWorker::~LauncherWorker()
{}

void LauncherWorker::Timer_Constructed()
{
printf("@@----- 01\n");
	QFile pa(_pfnParams);
	if (!pa.exists())
		{ g_pTheApp->exit(101); return;}
printf("@@----- 02\n");
	if (!pa.open(QIODevice::ReadOnly))
		{ g_pTheApp->exit(102); return;}
printf("@@----- 03\n");
	QByteArray ba = pa.readAll();
	if (ba.isEmpty())
		{ g_pTheApp->exit(103); return;}
	QString params(ba);
printf("@@----- 04\n");
	QStringList args = params.split(' ', QString::SkipEmptyParts);
printf("@@@@@@@@@@@@\n");
	int r = QProcess::execute(PathHelper::Instance()->OpenvpnPathfilename(), args);
printf("QProcess::execute() returns %d\n", r);
printf("@@@@@@@@@@@@\n");
	g_pTheApp->exit(r); return;
printf("@@----- 05\n");
}


