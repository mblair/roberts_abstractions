/*
 * File: xcompat.h
 * Version: 3.0
 * Last modified on Thu Sep 22 15:52:24 1994 by eroberts
 * -----------------------------------------------------
 * This interface exists entirely for portability with Unix systems
 * other than BSD 4.4, which is the assumed model for the rest of
 * the code.  To get the graphics library to run on other systems
 * (such as System V, POSIX, and so forth), the graphics library
 * approaches the portability problem by reimplementing the BSD
 * system calls and functions in terms of the corresponding
 * facilities on the target machine.  The advantage of this
 * approach is that the clients can work with only one model
 * that is relatively well understood.
 *
 * This interface has no effect unless one of the following macros
 * is defined:
 *
 *     HasPoll   -- Used if the System V poll call is available
 *
 * Additional conditionals may be added as new environments are
 * supported.
 */

#ifndef _xcompat_h
#define _xcompat_h

#ifdef HasPoll

#include <string.h>

#ifndef FOPEN_MAX
#  define FOPEN_MAX 128
#endif

#undef fd_set
#undef FD_ZERO
#undef FD_SET
#undef FD_CLR
#undef FD_ISSET

#define fd_set fdSetT
#define FD_ZERO(fdptr) (memset((fdptr)->fds, sizeof (fd_set), 0))
#define FD_SET(fd, fdptr) ((fdptr)->fds[fd] = 1)
#define FD_CLR(fd, fdptr) ((fdptr)->fds[fd] = 0)
#define FD_ISSET(fd, fdptr) ((fdptr)->fds[fd])

typedef struct {
    char fds[FOPEN_MAX];
} fdSetT;

#define select SimulateSelect

int SimulateSelect(int width,
                   fd_set *readfds,
                   fd_set *writefds,
                   fd_set *exceptfds,
                   struct timeval *tvp);

#endif /* HasPoll */

#endif
