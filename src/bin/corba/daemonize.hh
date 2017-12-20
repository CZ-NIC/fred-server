#ifndef DAEMONIZE_H
#define DAEMONIZE_H

/*simple daemonize for linux mainly from 
http://www.itp.uzh.ch/~dpotter/howto/daemonize
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DAE_OK 0
#define DAE_FAIL -1

static void daemonize(void)
{
    pid_t pid, sid;

    /* already a daemon */
    if ( getppid() == 1 ) return;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(DAE_FAIL);
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {
        exit(DAE_OK);
    }

    /* At this point we are executing as the child process */

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        exit(DAE_FAIL);
    }

    /* Change the current working directory.  This prevents the current
       directory from being locked; hence not being able to remove it. */
    if ((chdir("/")) < 0) {
        exit(DAE_FAIL);
    }


    /* Redirect standard files to /dev/null */
    if(freopen( "/dev/null", "r", stdin) == NULL) exit(DAE_FAIL);
    if(freopen( "/dev/null", "w", stdout) == NULL) exit(DAE_FAIL);
    if(freopen( "/dev/null", "w", stderr) == NULL) exit(DAE_FAIL);
}

#endif //DAEMONIZE_H
