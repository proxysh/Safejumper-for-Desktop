#include "thread_oldip.h"
#include "authmanager.h"
#include "stun.h"

void Thread_OldIp::run()
{
    QString ip;
	uint64_t v = AuthManager::Instance()->GetRnd64();
	sockaddr_in sa;
	const char * sv;

	uint8_t z[4];
	memset(z, 0, sizeof(z));
    int re = GetExternalIPbySTUN(v, &sa, &sv);

	if (re > -1)
	{
		memcpy(z, &sa.sin_addr, 4);
fprintf(stderr, "OldIp::run(): %u.%u.%u.%u", z[0], z[1], z[2], z[3]);
		ip.sprintf("%u.%u.%u.%u", z[0], z[1], z[2], z[3]);
	}

    emit resultReady(ip);
}
