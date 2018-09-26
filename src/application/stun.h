#ifndef STUN_H
#define STUN_H



#ifndef sockaddr_in

// from /usr/include/netinet/in.h

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint32_t
#define uint32_t unsigned long

// TODO: -1 linux: ensure 32bit unsigned
//#if sizeof(uint32_t) != 4
//#error Cannot define  uint32_t !
//#endif
#endif

#ifndef uint64_t
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <time.h>
#include <errno.h>

#else

#ifndef sa_family_t
typedef uint8_t		sa_family_t;
#endif

#ifndef in_port_t
typedef	uint16_t		in_port_t;
#endif

#ifndef in_addr_t
typedef	uint32_t	in_addr_t;
#endif


#endif

//#ifdef Q_OS_DARWIN
//struct in_addr {
//	in_addr_t s_addr;
//};
//#endif

/*
 * Socket address, internet style.
 */
//#ifdef Q_OS_DARWIN
//struct sockaddr_in {
//	uint8_t	sin_len;
//	sa_family_t	sin_family;
//	in_port_t	sin_port;
//	struct	in_addr sin_addr;
//	char		sin_zero[8];
//};
//#endif

#endif

int GetExternalIPbySTUN(uint64_t rnd, struct sockaddr_in *mapped, const char **srv);

#endif // STUN_H

