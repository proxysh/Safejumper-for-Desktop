/***************************************************************************
 *   Copyright (C) 2017 by Jeremy Whiting <jeremypwhiting@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2 of the License.                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "thread_oldip.h"

#include <stdint.h>

#include "authmanager.h"
#include "stun.h"

void Thread_OldIp::run()
{
    QString ip;
    uint64_t v = AuthManager::instance()->getRandom64();
    sockaddr_in sa;
    const char * sv;

    uint8_t z[4];
    memset(z, 0, sizeof(z));
    int re = GetExternalIPbySTUN(v, &sa, &sv);

    if (re > -1) {
        memcpy(z, &sa.sin_addr, 4);
        fprintf(stderr, "OldIp::run(): %u.%u.%u.%u\n", z[0], z[1], z[2], z[3]);
        ip.sprintf("%u.%u.%u.%u", z[0], z[1], z[2], z[3]);
    }

    emit resultReady(ip);
}
