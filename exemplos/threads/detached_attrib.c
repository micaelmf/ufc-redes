/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2018.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* detached_attrib.c

   An example of the use of POSIX thread attributes (pthread_attr_t):
   creating a detached thread.
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static void *
threadFunc(void *x)
{
    return x;
}

int
main(int argc, char *argv[])
{
    pthread_t thr;
    pthread_attr_t attr;
    int s;

    s = pthread_attr_init(&attr);       /* Assigns default values */
    if (s != 0)
        fputs("Error: pthread_attr_init", stderr);

    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (s != 0)
        fputs("Error: pthread_attr_setdetachstate", stderr);

    s = pthread_create(&thr, &attr, threadFunc, (void *) 1);
    if (s != 0)
        fputs("Error: pthread_create", stderr);

    s = pthread_attr_destroy(&attr);    /* No longer needed */
    if (s != 0)
        fputs("Error: pthread_attr_destroy", stderr);

    s = pthread_join(thr, NULL);
    if (s != 0)
        fputs("Error: pthread_join failed as expected\n", stderr);

    exit(EXIT_SUCCESS);
}
