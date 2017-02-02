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

#include "flag.h"

struct CountryDef {
    const char * shortname;
    const char * flagpng;
    int x;
    int y;
};

//  https://countrycode.org/
static CountryDef gs_countries [] = {
    {"Australia", "AU", 247, 107},
    {"Austria", "AT", 0, 0},
    {"Belgium", "BE", 0, 0},

    {"Brasil", "BR", 80, 92},
    {"Brazil", "BR", 80, 92},                       // handle typo

    {"Bulgaria", "BG", 156, 34},
    {"Canada", "CA", 37, 25},
    {"Chile", "CL", 64, 108},
    {"China", "CN", 223, 55},
    {"Costa Rica", "CR", 45, 70},
    {"Czech", "CZ", 0, 0},
    {"Estonia", "EE", 150, 25},
    {"Finland", "FI", 150, 22},
    {"France", "FR", 135, 35},
    {"Germany", "DE", 138, 32},
    {"Hong Kong", "HK", 230, 63},
    {"Hungary", "HU", 156, 34},
    {"Iceland", "IS", 108, 13},
    {"Ireland", "IE", 118, 27},
    {"Israel", "IL", 156, 51},
    {"India", "IN", 197, 65},
    {"Isle of Man", "IM", 118, 27},
    {"Italy", "IT", 136, 42},
    {"Japan", "JP", 255, 49},
    {"Kyrgyzstan", "KG", 183, 45},
    {"Latvia", "LV", 150, 25},

    {"Lichtenstein", "LI", 0, 0},                       // handle typo
    {"Liechtenstein", "LI", 0, 0},

    {"Lithuania", "LT", 150, 25},
    {"Luxembourg", "LU", 0, 0},
    {"Moldova", "MD", 156, 34},
    {"Netherlands", "NL", 0, 0},
    {"New Zealand", "NZ", 280, 125},
    {"Norway", "NO", 140, 20},
    {"Pakistan", "PK", 186, 58},
    {"Panama", "PA", 45, 70},
    {"Poland", "PL", 150, 25},
    {"Portugal", "PT", 125, 45},
    {"Romania", "RO", 156, 34},
    {"Russia", "RU", 210, 20},
    {"Serbia", "RS", 0, 0},
    {"Singapore", "SG", 230, 63},
    {"Slovakia", "SI", 0, 0},
    {"South Africa", "ZA", 148, 115},
    {"Spain", "ES", 125, 45},
    {"Sweden", "SE", 145, 22},
    {"Switzerland", "CH", 0, 0},
    {"Taiwan", "TW", 230, 65},
    {"Thailand", "TH", 220, 68},
    {"Turkey", "TR", 153, 46},
    {"U.S.", "US", 37, 47},
    {"Ukraine", "UA", 158, 34},
    {"United Kingdom", "GB", 118, 27},
    {"United States", "US", 37, 47},
};


QString flag::IconFromSrvName(const QString & srv)
{
    QString f = "_unknown";
    int id = IdFromName(srv);
    if (id > -1)
        f = gs_countries[id].flagpng;
    return f;
}

QPoint flag::CoordsFromSrvName(const QString & srv)
{
    QPoint p;
    int id = IdFromName(srv);
    if (id < 0)
        id = IdFromName("Germany");     // set default to the middle of Europe
    if (id > -1) {
        p.setX(gs_countries[id].x);
        p.setY(gs_countries[id].y);
    }
    return p;
}

QString flag::ShortName(const QString & name)
{
    QString s;
    int id = IdFromName(name);
    if (id > -1) {
        s = gs_countries[id].shortname;
    } else {
        s = name;
        s.remove("Hub", Qt::CaseInsensitive);
        s.remove("Boost", Qt::CaseInsensitive);
        s = s.trimmed();
    }
    return HandleTypo(s);
}

QString flag::ClearName(const QString & name)
{
    QString c = name;
    int p = name.indexOf(" Hub");
    if (p > -1) {
        c = name.mid(0, p);
    } else {
        for (p = name.length() - 1; p > 0; --p) {
            if (!(name[p].isDigit() || name[p].isSpace()))
                break;
        }
        if (p != (name.length() - 1))
            c = name.mid(0, p + 1);
    }

    return HandleTypo(c);
}

int flag::IdFromName(const QString & srv)
{
    int id = -1;
    for (size_t k = 0, sz = (sizeof(gs_countries)/sizeof(gs_countries[0])); k < sz; ++k) {
        if (srv.contains(gs_countries[k].shortname, Qt::CaseInsensitive)) {
            id = k;
            break;
        }
    }
    return id;
}

struct substitution_pair_t {
    const char * wrong;
    const char * correct;
};

static substitution_pair_t gs_Substitutions [] = {
    {"Brasil", "Brazil"},
    {"Lichtenstein", "Liechtenstein"},
};

QString flag::HandleTypo(const QString & name)
{
    QString s = name;
    for (size_t k = 0, sz = sizeof(gs_Substitutions)/sizeof(gs_Substitutions[0]); k < sz; ++k) {
        if (s.indexOf(gs_Substitutions[k].wrong) > -1)
            s = s.replace(gs_Substitutions[k].wrong, gs_Substitutions[k].correct);
    }
    return s;
}


