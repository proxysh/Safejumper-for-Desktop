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

#include "common.h"

#include <QUrl>
#include <QDesktopServices>
#include <QSettings>
#include <QSslConfiguration>
#include <QSslSocket>

bool IsValidIp(const QString & ip)
{
    bool valid = false;
    int points = ip.count('.');
    if (3 == points) {
        // do parse
        QStringList ar = ip.split('.', QString::SkipEmptyParts);
        if (ar.length() == 4) {
            bool ok = true;
            for (int k = 0; k < 4 && ok; ++k) {
                int v = ar[k].toInt(&ok);
                if (!(0 <= v && v <= 255)) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                valid = ok;
        }
    }
    return valid;
}


bool OpenUrl(const char * url)
{
    return QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));;
}

bool OpenUrl_Support()
{
    return OpenUrl("https://proxy.sh/support");
}

bool OpenUrl_Panel()
{
    return OpenUrl("https://proxy.sh/panel");
}

bool OpenUrl_Earn()
{
    return OpenUrl("https://proxy.sh/money");
}

bool OpenUrl_Bug()
{
    return OpenUrl("https://proxy.sh/panel/knowledgebase/27/Troubleshooting");
    //https://proxy.sh/panel/submitticket.php
}

bool launchUpdateUrl()
{
    return OpenUrl("https://proxy.sh/software");
}

// maybe empty str >>> false
// true only for numberic: [1-65535]
bool IsValidPort(const QString & s)
{
    bool ok;
    int v = s.toInt(&ok);
    if (ok)
        if (v < 1 || v > 65535)
            ok = false;
    return ok;
}

void SaveCb(const char * name, bool val)
{
    QSettings settings;
    settings.setValue(name, val);
}

QString EscapePsw(const QString & raw)
{
    QString s;
    for (int k = 0; k < raw.length(); ++k) {
        if (raw.at(k) == '\\' ) {
            s += "\\\\";
        } else {
            if (raw.at(k) == '"') {
                s += "\\\"";
            } else {
                s += raw.at(k);
            }
        }
    }
    return s;
}

QNetworkRequest BuildRequest(const QUrl & u)
{
    QNetworkRequest req(u);
    QSslConfiguration conf = req.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    req.setSslConfiguration(conf);
    return req;
}

