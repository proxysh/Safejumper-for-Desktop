#include "pingwaiter.h"
#include "authmanager.h"

PingWaiter::PingWaiter(size_t idWaiter, QObject *parent)
	: QObject(parent)
	, _idWaiter(idWaiter)
{}

PingWaiter::~PingWaiter()
{}

void PingWaiter::PingFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	AuthManager::Instance()->PingComplete(_idWaiter);
}

void PingWaiter::PingError(QProcess::ProcessError)
{
	AuthManager::Instance()->PingErr(_idWaiter);
}

void PingWaiter::Timer_Terminate()
{
	AuthManager::Instance()->PingTerminate(_idWaiter);
}


