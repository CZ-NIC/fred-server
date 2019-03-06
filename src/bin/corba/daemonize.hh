/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef DAEMONIZE_HH_E494C4E4E86C4BF9A0044B2654DE3D74
#define DAEMONIZE_HH_E494C4E4E86C4BF9A0044B2654DE3D74

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

#endif
