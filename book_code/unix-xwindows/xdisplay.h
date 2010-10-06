/*
 * File: xdisplay.h
 * Version: 3.0
 * Last modified on Sat Oct  1 12:25:08 1994 by eroberts
 * -----------------------------------------------------
 * This interface is responsible for the actual display operations
 * of the X manager phase of the graphics.c implementation.  In many
 * cases, the functions in this interface are similar to those in
 * the client interface to the graphics library.  The graphics.h
 * interface, for example, contains DrawLine and DrawArc; this
 * interface exports XDDrawLine and XDDrawArc.  The difference
 * is that the graphics.h functions simply send commands down a
 * communication channel, while the xdisplay.h functions actually
 * perform the rendering operations for the X window manager.
 */

#ifndef _xdisplay_h
#define _xdisplay_h

#include "genlib.h"

/*
 * Function: XDOpenDisplay
 * Usage: XDOpenDisplay(title);
 * ----------------------------
 * This function creates a display window and performs all the
 * necessary initialization for the X system.  The title string
 * is displayed in the title bar.
 */

void XDOpenDisplay(string title);

/*
 * Function: XDCloseDisplay
 * Usage: XDCloseDisplay();
 * ------------------------
 * This function closes the graphics window and frees all of the
 * X storage.
 */

void XDCloseDisplay(void);

/*
 * Function: XDDisplayFD
 * Usage: fd = XDDisplayFD();
 * --------------------------
 * This function returns the Unix file descriptor of the X
 * connection to the graphics window display.
 */

int XDDisplayFD(void);

/*
 * Function: XDProcessXEvent
 * Usage: if (XDProcessXEvent()) . . .
 * -----------------------------------
 * This function checks to see if there is a pending X event
 * and, if so, processes that event.  The function returns
 * TRUE if an event was detected and FALSE if no events were
 * pending so that the client can set a timeout before testing
 * again for another event.
 */

bool XDProcessXEvent(void);

/*
 * Function: XDCheckForRedraw
 * Usage: XDCheckForRedraw();
 * --------------------------
 * This function checks to see if the window needs redrawing and,
 * if so, issues the X commands to update the screen.
 */

void XDCheckForRedraw(void);

/*
 * Function: XDSetRedrawFlag
 * Usage: XDSetRedrawFlag();
 * -------------------------
 * This function informs the X display module that the screen needs
 * to be redrawn.
 */

void XDSetRedrawFlag(void);

/*
 * Function: XDClearDisplay
 * Usage: XDClearDisplay();
 * ------------------------
 * XDClearDisplay erases the entire contents of the graphics window.
 */

void XDClearDisplay(void);

/*
 * Function: XDDrawLine
 * Usage: XDDrawLine(x, y, dx, dy);
 * --------------------------------
 * This function draws a line from (x, y) to (x+dx, y+dy).
 */

void XDDrawLine(double x, double y, double dx, double dy);

/*
 * Function: XDDrawArc
 * Usage: XDDrawArc(x, y, rx, ry, start, sweep);
 * ---------------------------------------------
 * This function draws an elliptical arc segment centered at
 * (x, y) with cartesian radii rx and ry.  Note that the
 * interpretation of (x, y) is not the current point as
 * it is in the DrawArc call.  The start and sweep parameters
 * are interpreted as they are in DrawArc.
 */

void XDDrawArc(double x, double y, double rx, double ry,
                double start, double sweep);

/*
 * Function: XDDrawText
 * Usage: XDDrawText(x, y, text);
 * ------------------------------
 * This function displays the string text at (x, y) using the
 * current font and size.
 */

void XDDrawText(double x, double y, string text);

/*
 * Function: XDTextWidth
 * Usage: width = XDTextWidth(text);
 * ---------------------------------
 * This function returns the width of the text string at the
 * current font and size.
 */

double XDTextWidth(string text);

/*
 * Function: XDSetFont
 * Usage: str = XDSetFont(font, size, style);
 * ------------------------------------------
 * This function finds the appropriate font/size/style combination
 * based on the user's request and returns a string consisting of
 * the size, style, and font names each separated by a space.
 */

string XDSetFont(string font, int size, int style);

/*
 * Function: XDSetTitle
 * Usage: XDSetTitle(title);
 * -------------------------
 * This function sets the title bar for the window.
 */

void XDSetTitle(string title);

/*
 * Function: XDSetEraseMode
 * Usage: XDSetEraseMode(flag);
 * ----------------------------
 * This function sets the erase mode setting according to the
 * Boolean flag.
 */

void XDSetEraseMode(bool flag);

/*
 * Function: XDStartRegion
 * Usage: XDStartRegion(grayScale);
 * --------------------------------
 * The XDStartRegion function sets the internal state so
 * that subsequent calls to XDDrawLine and XDDrawArc (which
 * have already been checked by the client to ensure that they
 * form a connected path) are used to create a fillable polygon.
 * The region is displayed by the corresponding XDEndRegion.
 */

void XDStartRegion(double grayScale);

/*
 * Function: XDEndRegion
 * Usage: XDEndRegion();
 * ---------------------
 * This function closes the region opened by XDStartRegion.
 */

void XDEndRegion(void);

/*
 * Function: XDGetMouse
 * Usage: XDGetMouse(&buttonState, &x, &y);
 * ----------------------------------------
 * This function returns the current mouse state through the
 * argument pointers.
 */

void XDGetMouse(bool *buttonStateP, double *xp, double *yp);

/*
 * Function: XDWaitForMouse
 * Usage: XDWaitForMouse(buttonState);
 * -----------------------------------
 * This function waits for the mouse to enter the specified
 * state (down = TRUE);
 */

void XDWaitForMouse(bool buttonState);

/*
 * Function: XDSetWindowSize
 * Usage: XDSetWindowSize(width, height);
 * --------------------------------------
 * This function sets the width and height values of the window
 * and must be called prior to XDOpenDisplay.
 */

void XDSetWindowSize(double width, double height);

/*
 * Function: XDGetScreenSize
 * Usage: XDGetScreenSize(&screenWidth, &screenHeight);
 * ----------------------------------------------------
 * This function returns the screen size and may be called in the
 * following circumstances, each of which is unusual:
 *   (a) prior to the XDOpenDisplay call
 *   (b) from the client fork
 */

void XDGetScreenSize(double *pScreenWidth, double *pScreenHeight);

/*
 * Function: XDGetResolution
 * Usage: XDGetResolution(&xdpi, &ydpi);
 * -------------------------------------
 * This function returns the screen resolution, possibly modified by
 * the scale reduction.  Like XDGetScreenSize, this function
 * can be called prior to the XDOpenDisplay call or from the client
 * fork.
 */

void XDGetResolution(double *pXDPI, double *pYDPI);

/*
 * Function: XDGetNColors
 * Usage: ncolors = XDGetNColors();
 * --------------------------------
 * This function returns the number of colors supported by the screen.
 * This function can be called prior to the XDOpenDisplay call or from
 * the client fork.
 */

int XDGetNColors(void);

/*
 * Function: XDSetColor
 * Usage: XDSetColor(red, green, blue);
 * ------------------------------------
 * This function sets the pen color as specified by the arguments.
 */

void XDSetColor(double red, double green, double blue);

#endif
