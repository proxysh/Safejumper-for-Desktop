#include "common.h"
#include <sys/unistd.h>
#ifdef Q_OS_DARWIN
#include <unistd.h>
#endif
#include <vector>
#include <string>
#include <cstdio>

#include <QString>
#include <QVector>

int exec_fork(const QString & pfnOV, const QStringList & args)
{
    pid_t processId = 0;
    printf("@@@ parent launcher  pid = %d\n", processId);

    if ((processId = fork()) == 0) {
        // child: start OpenVPN here
        printf("@@@ CHILD launcher  pid = %d\n", processId);

        std::string bufapp = pfnOV.toStdString();
        const char * app = bufapp.c_str();

        QVector<QString> v0 = args.toVector();
        std::vector<std::string> buf;
        buf.push_back(app);
        for (int k = 0; k < v0.length(); ++k)
            buf.push_back(v0[k].toStdString());

        std::vector<char *> argvbuf;
        for (size_t k = 0; k < buf.size(); ++k) {
            const char * p0 = buf.at(k).c_str();
            char * p1 = const_cast<char*>(p0);
            argvbuf.push_back(p1);
        }
        argvbuf.push_back(NULL);
        printf("@@ cmd = %s\n", app);
        printf("@@ before execv()\n");

        char * const * argv = &argvbuf[0];
        if (execv(app, argv) < 0)                               // never return on success
            //if (execv(app, &argvbuf[0]) < 0)              // never return on success
        {
            printf("@@  execv() < 0\n");
            perror("execv error");
            printf("@@  execv() < 0\n");
        }
        printf("@@ after execv()\n");

    } else {    // fork()
        // parent launcher
        printf("@@@ parent launcher  pid = %d\n", processId);
        if (processId < 0) {
            perror("@@@@@@ fork() error ###########");
        } else {
            printf("@@@@@@ fork() parent EXIT_SUCCESS  pid = %d\n", processId);
            return EXIT_SUCCESS;
        }
    }
    perror("EXIT_FAILURE");
    return EXIT_FAILURE;

}

void become_root()
{
    uid_t uidBefore  = getuid();
    uid_t euidBefore = geteuid();
    printf("@@ uidBefore = %d, euidBefore = %d, %s\n", uidBefore, euidBefore, (uidBefore == euidBefore ? "EQUAL" : "NOT EQUAL") );

    if ( (uidBefore != 0) || (euidBefore != 501))
        fprintf(stderr, "@@ Error: become root: Not root and not non-root\n");

    if (euidBefore != 0) {
        int result = seteuid(0);
        if (result != 0)
            fprintf(stderr, "Error: Unable to become root, res = %d\n", result);
        else
            printf("@@seteuid(0), res = %d\n", result);
        printf("@@ euid = %d\n", geteuid());
    }

    if (uidBefore != 0) {
        int r3 = setuid(0);
        if (r3 != 0)
            fprintf(stderr, "Error: Unable to setuid(0), res = %d\n", r3);
        else
            printf("@@setuid(0), res = %d\n", r3);
    }

    printf("@@ uid = %d\n", getuid());
    printf("@@ euid = %d\n", geteuid());
}

