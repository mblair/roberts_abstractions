/*
 * File: xcompat.c
 * Version: 3.0
 * Last modified on Sat Oct  1 11:28:05 1994 by eroberts
 * -----------------------------------------------------
 * This implementation and the corresponding interface simulate the
 * operation of any BSD4.2 calls that are required by the graphics
 * package but that are not defined in the local system.  See the
 * interface for details.  Note that this file generates no code
 * unless one of the conditions is set.
 */

#ifdef HasPoll

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include "xcompat.h"

/* Private function prototypes */

static void SigAlarmHandler();

/*
 * Function: SimulateSelect
 * ------------------------
 * This function simulates the BSD select call by mapping it onto
 * the poll system call implemented under System V.
 */

int SimulateSelect(int width,
                   fd_set *readfds,
                   fd_set *writefds,
                   fd_set *exceptfds,
                   struct timeval *tvp)
{
    struct pollfd fds[FOPEN_MAX];
    int i, fd, nfds;
    short events;
    int timeout, nready;

    nfds = 0;
    if (tvp == NULL) {
        timeout = 0;
    } else {
        timeout = tvp->tv_sec * 1000 + (tvp->tv_usec + 500) / 1000;
    }
    for (fd = 0; fd < width; fd++) {
        events = 0;
        if (readfds && FD_ISSET(fd, readfds)) {
            events |= POLLIN;
            FD_CLR(fd, readfds);
        }
        if (writefds && FD_ISSET(fd, writefds)) {
            events |= POLLOUT;
            FD_CLR(fd, writefds);
        }
        if (exceptfds && FD_ISSET(fd, exceptfds)) {
            events |= POLLIN | POLLOUT;
            FD_CLR(fd, exceptfds);
        }
        if (events != 0) {
            fds[nfds].fd = fd;
            fds[nfds].events = events;
            fds[nfds].revents = 0;
            nfds++;
        }
    }
    nready = poll(fds, nfds, timeout);
    if (nready <= 0) return (nready);
    for (i = 0; i < nfds; i++) {
        if (fds[i].revents & (POLLIN | POLLPRI | POLLHUP)) {
            FD_SET(fds[i].fd, readfds);
        }
        if (fds[i].revents & POLLOUT) {
            FD_SET(fds[i].fd, writefds);
        }
        if (fds[i].revents & (POLLERR | POLLNVAL)) {
            FD_SET(fds[i].fd, exceptfds);
        }
    }
    return (nready);
}

#endif
