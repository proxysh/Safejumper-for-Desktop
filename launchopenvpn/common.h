#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QStringList>

int exec_fork(const QString & pfnOV, const QStringList & args);
void become_root();

#endif // COMMON_H
