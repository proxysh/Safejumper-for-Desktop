#ifndef FLAG_H
#define FLAG_H

#include <QString>
#include <QPoint>

class flag
{
public:
    static QString IconFromSrvName(const QString & srv);
    static QPoint CoordsFromSrvName(const QString & srv);

    // name from server into short name to display on map control
    // e.g.
    // U.S. Florida Hub => U.S.
    static QString ShortName(const QString & name);

    // clear name from "Hub", "Boost", and trailing server number
    // U.S. Florida Hub => U.S. Florida
    // Japan 2 => Japan
    static QString ClearName(const QString & name);
private:
    static int IdFromName(const QString & srv);
    static QString HandleTypo(const QString & name);                    // handle typo in server names: e.g. Brasil => Brazil
};

#endif // FLAG_H
