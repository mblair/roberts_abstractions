/*
 * File: xmanager.c
 * Version: 3.0
 * Last modified on Sat Oct  1 12:28:02 1994 by eroberts
 * -----------------------------------------------------
 * This file implements the xmanager.h interface, which is responsible
 * for the X side of the communication between the graphics client and
 * the X manager.  The xmanager module handles the process management
 * and intermodule communication.  All work involving actual structures
 * is performed by the xdisplay module.
 */

/*
 * General implementation notes
 * ----------------------------
 * As with all implementations of the graphics library, the X
 * implementation is complicated by the change in model.  The X
 * programming paradigm is based on the idea of an "event loop,"
 * in which the application program continually calls XNextEvent
 * to determine whether any activity is required.  This strategy
 * runs counter to the more conventional view of a main program
 * as the highest level in the problem decomposition.  In the
 * context of teaching programming, we want the main program and
 * its subprograms to draw the figures -- a strategy which is not
 * easy directly compatible with the X paradigm.
 *
 * To solve this problem, the X implementation creates two forks:
 * one to poll for X events and manage the screen and the other
 * to run the user program.  These processes are established by
 * the first call to InitGraphics and remain in place until the
 * program terminates.  So that program termination can be
 * handled correctly, the child process is the one that returns
 * to the InitGraphics caller where it runs the client program.
 * The parent process, which is called the X manager process,
 * never returns from InitGraphics.  After initializing the
 * display, the X manager process runs an event loop waiting
 * simultaneously for graphical events and commands from the
 * client process.  All of this mechanism is invisible to the
 * client.
 *
 * The two processes communicate by means of pipes running in
 * each direction.  The client communicates with the X manager by
 * sending commands over its output pipe.  The command format
 * consists of ASCII text lines in the following format:
 *
 *       LLLCC XXXXXX
 *
 * where LLL is a three-digit length field giving the length of
 * the entire line (including the newline), CC is a two-digit
 * field specifying the command number and XXXXXX is an arbitrary
 * text string specifying arguments to the command.  The length
 * of the text string can be determined from the line length
 * indicator.
 *
 * Some client operations require a response from the X manager.
 * These operations call XMGetResponse() to read a line from the
 * client.  Because this operation is synchronous, there is no
 * need for the more cumbersome form used in the opposite
 * direction.
 *
 * This interface is used by both the client side (graphics.c)
 * and the X manager side (xmanager.c), but it is important to
 * remember that the two are running in different forks.  For
 * each fork, the inPipe and outPipe variables correspond to
 * the local perspective.  Thus, reading is always performed
 * on inPipe and writing is performed on outPipe.  The ends
 * of the pipe are crossed between the processes so that data
 * that is written to outPipe by one fork appears in inPipe
 * on the other side.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>

#include "genlib.h"
#include "exception.h"
#include "simpio.h"
#include "xmanager.h"
#include "xdisplay.h"
#include "xcompat.h"
#include "glibrary.h"

/* Constants */

#define ClientTimeout 0.05

/* Private state variables */

static FILE *inPipe, *outPipe;
static int infd;
static pid_t child;

static char cmdBuffer[CommandBufferSize];

static bool exitGraphicsFlag;

/* Private function prototypes */

static void MainEventLoop(void);
static bool ReadMessage(void);
static void ProcessMessage(void);
static void LineMessage(string args);
static void ArcMessage(string args);
static void TextMessage(string args);
static void WidthMessage(string args);
static void FontMetricsMessage(string args);
static void SetEraseMessage(string args);
static void StartRegionMessage(string args);
static void EndRegionMessage(string args);
static void ClearMessage(string args);
static void UpdateMessage(string args);
static void SetTitleMessage(string args);
static void SetFontMessage(string args);
static void GetMouseMessage(string args);
static void WaitForMouseMessage(string args);
static void SetColorMessage(string args);

/* Exported entries */

void XMInitialize(string title)
{
    int p1[2], p2[2];

    XDOpenDisplay(title);
    pipe(p1);
    pipe(p2);
    if ((child = fork()) == 0) {
        close(p1[0]);
        close(p2[1]);
        inPipe = fdopen(p2[0], "r");
        if (inPipe == NULL) Error("Can't open pipe from X client");
        outPipe = fdopen(p1[1], "w");
        if (outPipe == NULL) Error("Can't open pipe to X client");
    } else {
        close(p1[1]);
        close(p2[0]);
        infd = p1[0];
        outPipe = fdopen(p2[1], "w");
        if (outPipe == NULL) Error("Can't open output pipe in X client");
        exitGraphicsFlag = FALSE;
        try {
            MainEventLoop();
          except(ErrorException)
            fprintf(stderr, "Error: %s\n", (string) GetExceptionValue());
            XDCloseDisplay();
            kill(child, SIGKILL);
            exit(1);
        } endtry
        (void) waitpid(child, NULL, 0);
        if (!exitGraphicsFlag) {
            printf("Press return to exit.\n");
            infd = 0;
            MainEventLoop();
        }
        XDCloseDisplay();
        exit(0);
    }
}

void XMSendCommand(commandT cmd, string args)
{
    int len;

    len = strlen(args) + 4;
    fprintf(outPipe, "%3d%2d %s\n", len, (int) cmd, args);
    fflush(outPipe);
}

void XMGetResponse(char buffer[])
{
    fgets(buffer, CommandBufferSize - 1, inPipe);
}

void XMReleaseClient(void)
{
    fprintf(outPipe, "\n");
    fflush(outPipe);
}

/* Private functions */

static void MainEventLoop(void)
{
    struct timeval tv, *tvp;
    int xfd, width, sc;
    fd_set readmask, readset;
    string message;

    xfd = XDDisplayFD();
    tv.tv_sec = ClientTimeout;
    tv.tv_usec = ((int) (ClientTimeout * 1000000)) % 1000000;
    FD_ZERO(&readmask);
    FD_SET(xfd, &readmask);
    FD_SET(infd, &readmask);
    width = GLMax(xfd, infd) + 1;
    tvp = &tv;
    while (TRUE) {
        if (XDProcessXEvent()) {
            tvp = &tv;
        } else {
            readset = readmask;
            sc = select(width, &readset, NULL, NULL, tvp);
            tvp = (sc == 0) ? NULL : &tv;
            if (sc == 0) {
                XDCheckForRedraw();
            } else if (FD_ISSET(infd, &readset)) {
                if (!ReadMessage()) {
                    XDCheckForRedraw();
                    return;
                }
                ProcessMessage();
                XDSetRedrawFlag();
            }
        }
    }
}

static bool ReadMessage(void)
{
    int len;
    char lbuf[4];

    if (infd == 0) {
        FreeBlock(GetLine());
        return (FALSE);
    }
    if (read(infd, lbuf, 3) == 0) return (FALSE);
    lbuf[3] = '\0';
    len = atoi(lbuf);
    if (read(infd, cmdBuffer, len) != len) {
        Error("Unexpected end of file");
    }
    if (cmdBuffer[len-1] != '\n') {
        Error("Unexpected end of command");
    }
    cmdBuffer[len-1] = '\0';
    return (TRUE);
}

static void ProcessMessage(void)
{
    int id;
    string args;

    args = cmdBuffer + 3;
    sscanf(cmdBuffer, "%d", &id);
    switch ((commandT) id) {
      case LineCmd: LineMessage(args); break;
      case ArcCmd: ArcMessage(args); break;
      case TextCmd: TextMessage(args); break;
      case WidthCmd: WidthMessage(args); break;
      case FontMetricsCmd: FontMetricsMessage(args); break;
      case SetEraseCmd: SetEraseMessage(args); break;
      case StartRegionCmd: StartRegionMessage(args); break;
      case EndRegionCmd: EndRegionMessage(args); break;
      case ClearCmd: ClearMessage(args); break;
      case UpdateCmd: UpdateMessage(args); break;
      case SetTitleCmd: SetTitleMessage(args); break;
      case SetFontCmd: SetFontMessage(args); break;
      case GetMouseCmd: GetMouseMessage(args); break;
      case WaitForMouseCmd: WaitForMouseMessage(args); break;
      case SetColorCmd: SetColorMessage(args); break;
      case ExitGraphicsCmd: exitGraphicsFlag = TRUE; break;
      default: Error("Internal error: Illegal command"); break;
    }
}

static void LineMessage(string args)
{
    int n;
    double x, y, dx, dy;

    n = sscanf(args, "%lg %lg %lg %lg", &x, &y, &dx, &dy);
    if (n != 4) {
        Error("Internal error: Bad arguments to LineCmd");
    }
    XDDrawLine(x, y, dx, dy);
}

static void ArcMessage(string args)
{
    int n;
    double x, y, rx, ry, start, sweep;

    n = sscanf(args, "%lg %lg %lg %lg %lg %lg",
               &x, &y, &rx, &ry, &start, &sweep);
    if (n != 6) {
        Error("Internal error: Bad arguments to ArcCmd");
    }
    XDDrawArc(x, y, rx, ry, start, sweep);
}

static void TextMessage(string args)
{
    int n;
    char *space;
    double x, y;

    n = sscanf(args, "%lg %lg", &x, &y);
    if (n != 2) {
        Error("Internal error: Bad arguments to TextCmd");
    }
    space = strchr(strchr(args, ' ') + 1, ' ');
    if (space == NULL) {
        Error("Internal error: Bad arguments to TextCmd");
    }
    XDDrawText(x, y, space + 1);
}

static void WidthMessage(string args)
{
    double width;

    width = XDTextWidth(args);
    fprintf(outPipe, "%.12g\n", width);
    fflush(outPipe);
}

static void FontMetricsMessage(string args)
{
    double ascent, descent, height;

    DisplayFontMetrics(&ascent, &descent, &height);
    fprintf(outPipe, "%.12g %.12g %.12g\n", ascent, descent, height);
    fflush(outPipe);
}

static void SetEraseMessage(string args)
{
    int n, flag;

    n = sscanf(args, "%d", &flag);
    if (n != 1) {
        Error("Internal error: Bad arguments to SetEraseCmd");
    }
    XDSetEraseMode((bool) flag);
}

static void StartRegionMessage(string args)
{
    int n;
    double grayScale;

    n = sscanf(args, "%lg", &grayScale);
    if (n != 1) {
        Error("Internal error: Bad arguments to SetEraseCmd");
    }
    XDStartRegion(grayScale);
}

static void EndRegionMessage(string args)
{
    XDEndRegion();
}

static void ClearMessage(string args)
{
    XDClearDisplay();
}

static void UpdateMessage(string args)
{
    XDSetRedrawFlag();
    XDCheckForRedraw();
}

static void SetTitleMessage(string args)
{
    XDSetTitle(args);
}

static void SetFontMessage(string args)
{
    int n, size, style;
    char *space;

    n = sscanf(args, "%d %d", &size, &style);
    if (n != 2) {
        Error("Internal error: Bad arguments to SetFontCmd");
    }
    space = strchr(args, ' ');
    if (space == NULL) {
        Error("Internal error: Bad arguments to TextCmd");
    }
    space = strchr(space + 1, ' ');
    if (space == NULL) {
        Error("Internal error: Bad arguments to TextCmd");
    }
    fprintf(outPipe, "%s\n", XDSetFont(space + 1, size, style));
    fflush(outPipe);
}

static void GetMouseMessage(string args)
{
    bool down;
    double x, y;

    XDGetMouse(&down, &x, &y);
    fprintf(outPipe, "%d, %.12g, %.12g\n", down, x, y);
    fflush(outPipe);
}

static void WaitForMouseMessage(string args)
{
    XDWaitForMouse(StringEqual(args, "D"));
}

static void SetColorMessage(string args)
{
    double red, green, blue;
    int n;

    n = sscanf(args, "%lg %lg %lg", &red, &green, &blue);
    if (n != 3) {
        Error("Internal error: Bad arguments to SetColorCmd");
    }
    XDSetColor(red, green, blue);
}
