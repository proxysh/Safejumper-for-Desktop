#include <cstdio>

#include "common.h"
#include "runit.h"

template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

static const char * cmds [] =
{
	"/usr/bin/easy_install pip",
	"/usr/local/bin/pip install obfsproxy"
};

int main(int argc, char *argv[])
{
	become_root();
	for (int k = 0; k < arraysize(cmds); ++k)
		runit(cmds[k], 120000);

	return 0;
}
