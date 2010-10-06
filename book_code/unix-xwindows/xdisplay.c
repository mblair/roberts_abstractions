/*
 * File: xdisplay.c
 * Version: 3.0
 * Last modified on Sat Oct  1 12:25:07 1994 by eroberts
 * -----------------------------------------------------
 * This file implements the xdisplay.h interface, which is
 * responsible for all of the X windows interaction.  Before
 * you attempt to understand this implementation, you need to
 * understand the basics of X11 library.
 */

/*
 * General implementation notes
 * ----------------------------
 * This implementation creates two X drawables: the graphics
 * window itself (mainWindow) and an offscreen window (osWindow).
 * All rendering is done into the offscreen window.  When update
 * events occur, the graphics window is updated by copying bits
 * from the offscreen window.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "genlib.h"
#include "strlib.h"
#include "glibrary.h"
#include "extgraph.h"
#include "xmanager.h"
#include "xdisplay.h"

/*
 * Parameters
 * ----------
 * RequiredMargin -- Minimum margin on each side of screen (inches)
 * BorderPixels   -- Width of the window border in pixels
 * MaxFontList    -- Size of the font list we will accept
 * PStartSize     -- Starting size for polygon (must be greater than 1)
 * DefaultFont    -- Font that serves as the "Default" font
 */

#define RequiredMargin   0.5
#define BorderPixels     1
#define MaxFontList    500
#define PStartSize      50
#define DefaultFont     "courier"

/*
 * Other constants
 * ---------------
 * Epsilon -- Small offset used to avoid banding/aliasing
 * GCFgBg  -- GC mask for both foreground and background
 */

#define Epsilon 0.000000001
#define GCFgBg (GCForeground | GCBackground)

/*
 * Static table: grayList
 * ----------------------
 * This table contains the bitmaps for the various gray-scale
 * values.  Adding more bitmaps to this list increases
 * the precision at which the client can specify gray scales.
 */

static char grayList[][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22 },
    { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA },
    { 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD },
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
};

#define NGrays (sizeof grayList / sizeof grayList[0])

/*
 * Type: waitStateT
 * ----------------
 * This type indicates the various conditions for which the
 * client might be waiting.
 */

typedef enum {
    NotWaiting,
    WaitingForMouseDown,
    WaitingForMouseUp
} waitStateT;

/*
 * Private variables
 * -----------------
 * displayIsOpen   -- TRUE if the display has been opened
 * redraw          -- TRUE if mainWindow needs redrawing
 * eraseMode       -- TRUE if erase mode has been set
 * xdpi, ydpi      -- Dots per inch in each coordinate
 * disp            -- X display containing windows
 * mainWindow      -- Handle of graphics window
 * osWindow        -- Handle of offscreen window bitmap
 * pixelDepth      -- Depth of pixels on the screen
 * colormap        -- Color map of screen
 * drawColor       -- Color used for drawing (black)
 * eraseColor      -- Color used for erasing (white)
 * mainGC          -- The graphics context (GC) for mainWindow
 * drawGC          -- The GC used for drawing on osWindow
 * eraseGC         -- The GC used for erasing on osWindow
 * grayGC          -- Array of GCs representing gray scales
 * grayStipple     -- Array of stipples used for gray scales
 * currentFont     -- Name of current font
 * currentSize     -- Current point size
 * currentStyle    -- Current style
 * fontInfo        -- Font structure pointer for current font
 * windowWidth     -- Width of the window in inches
 * windowHeight    -- Height of the window in inches
 * screenWidth     -- Width of the full screen in inches
 * screenHeight    -- Height of the full screen in inches
 * argV, argC      -- Used to set corresponding X window fields
 * waitState       -- Indicates what mouse event is awaited
 * regionStarted   -- TRUE is a region is in progress
 * regionGrayScale -- Gray scale density [0,1]
 * polygonPoints   -- Array of points used in current region
 * nPolygonPoints  -- Number of active points
 * polygonSize     -- Number of allocated points
 */

static bool displayIsOpen = FALSE;
static bool redraw = FALSE;
static bool eraseMode;
static double xdpi, ydpi;

static Display *disp;
static Window mainWindow;
static Pixmap osWindow;
static Colormap colormap;
static int pixelDepth;
static unsigned long drawColor, eraseColor;
static GC mainGC, drawGC, eraseGC;
static GC grayGC[NGrays];
static Pixmap grayStipple[NGrays];
static string currentFont;
static int currentSize;
static int currentStyle;
static XFontStruct *fontInfo;
static double windowWidth, windowHeight;
static double screenWidth, screenHeight;

static string argV[] = { "xdisplay" };
static int argC = sizeof argV / sizeof argV[0];

static waitStateT waitState;

static bool regionStarted;
static double regionGrayScale;
static XPoint *polygonPoints;
static int nPolygonPoints;
static int polygonSize;

/* Private function prototypes */

static void StartToOpenDisplay(void);
static void InitGC(void);
static void ForceRedraw(void);
static void RedrawWindow(void);
static void StartPolygon(void);
static void AddSegment(int x0, int y0, int x1, int y1);
static void DisplayPolygon(void);
static void RenderArc(double x, double y, double rx, double ry,
                      double start, double sweep);
static int SizeFromFontName(string fontName);
static double InchesX(int x);
static double InchesY(int y);
static int PixelsX(double x);
static int PixelsY(double y);
static int ScaleX(double x);
static int ScaleY(double y);

/* Exported entries */

/*
 * Function: XDOpenDisplay
 * -----------------------
 * The XDOpenDisplay function is relatively long but consists of
 * little more than a series of X calls to establish the
 * parameters of the display.
 */

void XDOpenDisplay(string title)
{
    XWMHints xwmh;
    XSizeHints xsh;
    XFontStruct *font;
    Screen *screen;
    XSetWindowAttributes xswa;
    long width, height;
    int bitmask, scnum;
    double xScale, yScale, scaleFactor;
    char geometry[100];

    StartToOpenDisplay();
    if ((font = XLoadQueryFont(disp, "fixed")) == NULL) {
        Error("Can't create font");
    }
    scnum = DefaultScreen(disp);
    screen = ScreenOfDisplay(disp, scnum);
    colormap = DefaultColormap(disp, scnum);
    drawColor = BlackPixel(disp, scnum);
    eraseColor = WhitePixel(disp, scnum);
    xScale = yScale = 1.0;
    if (windowWidth > screenWidth - 2 * RequiredMargin) {
        xScale = (screenWidth - 2 * RequiredMargin) / windowWidth;
    }
    if (windowHeight > screenHeight - 2 * RequiredMargin) {
        yScale = (screenHeight - 2 * RequiredMargin) / windowHeight;
    }
    scaleFactor = GLMinF(xScale, yScale);
    xdpi *= scaleFactor;
    ydpi *= scaleFactor;
    width = PixelsX(windowWidth);
    height = PixelsY(windowHeight);
    sprintf(geometry, "%dx%d+%d+%d", width, height,
            (DisplayWidth(disp, scnum) - width) / 2,
            (DisplayHeight(disp, scnum) - height) / 2);
    bitmask = XGeometry(disp, scnum, geometry, geometry,
                        BorderPixels,
                        font->max_bounds.width,
                        font->max_bounds.ascent + font->max_bounds.descent,
                        1, 1, &xsh.x, &xsh.y, &xsh.width, &xsh.height);
    if (bitmask & (XValue | YValue)) xsh.flags |= USPosition;
    if (bitmask & (WidthValue | HeightValue)) xsh.flags |= USSize;
    mainWindow = XCreateSimpleWindow(disp, DefaultRootWindow(disp),
                                     xsh.x, xsh.y, xsh.width, xsh.height,
                                     BorderPixels, drawColor, eraseColor);
    XSetStandardProperties(disp, mainWindow, title, title,
                           None, argV, argC, &xsh);
    xwmh.flags = (InputHint | StateHint);
    xwmh.input = False;
    xwmh.initial_state = NormalState;
    XSetWMHints(disp, mainWindow, &xwmh);
    xswa.colormap = colormap;
    xswa.bit_gravity = CenterGravity;
    XChangeWindowAttributes(disp, mainWindow,
                            CWColormap | CWBitGravity, &xswa);
    XSelectInput(disp, mainWindow,
                 ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow(disp, mainWindow);
    osWindow = XCreatePixmap(disp, DefaultRootWindow(disp),
                             xsh.width, xsh.height, pixelDepth);
    fontInfo = NULL;
    InitGC();
    regionStarted = FALSE;
    waitState = NotWaiting;
    XDClearDisplay();
    XFlush(disp);
}

/*
 * Function: XDCloseDisplay
 * ------------------------
 * This function frees the X structures allocated by the package.
 */

void XDCloseDisplay(void)
{
    int i;

    XDestroyWindow(disp, mainWindow);
    XFreePixmap(disp, osWindow);
    XFreeGC(disp, mainGC);
    XFreeGC(disp, drawGC);
    XFreeGC(disp, eraseGC);
    for (i = 0; i < NGrays; i++) {
        XFreeGC(disp, grayGC[i]);
        XFreePixmap(disp, grayStipple[i]);
    }
    XCloseDisplay(disp);
}

/*
 * Function: XDDisplayFD
 * ---------------------
 * This function simply returns the file descriptor for the X
 * connection.  The client cannot obtain this directly because the
 * display identified disp is private to this module.
 */

int XDDisplayFD(void)
{
    return (XConnectionNumber(disp));
}

/*
 * Function: XDProcessXEvent
 * -------------------------
 * This function is used to read and respond to a pending X event.
 * It returns TRUE if an event was processed.
 */

bool XDProcessXEvent(void)
{
    XEvent event;

    if (!XPending(disp)) return (FALSE);
    XNextEvent(disp, &event);
    if (event.xany.window == mainWindow) {
        switch (event.type) {
          case Expose:
            if (event.xexpose.count == 0) RedrawWindow();
            break;
          case ButtonPress:
            if (waitState == WaitingForMouseDown) {
                waitState = NotWaiting;
                XMReleaseClient();
            }
            break;
          case ButtonRelease:
            if (waitState == WaitingForMouseUp) {
                waitState = NotWaiting;
                XMReleaseClient();
            }
            break;
        }
    }
    return (TRUE);
}

/*
 * Function: XDCheckForRedraw
 * --------------------------
 * This function allows the client to specify that a quiescent point
 * has been achieved and that it would be a good time to redraw the
 * window.  A redraw occurs only if graphics updates have been made.
 */

void XDCheckForRedraw(void)
{
    if (redraw) ForceRedraw();
}

/*
 * Function: XDSetRedrawFlag
 * -------------------------
 * This function allows the client to indicate that the display has
 * changed and that a redraw operation should be performed when the
 * next call to XDCheckForRedraw occurs.  This mechanism currently
 * sets a single flag and should at some point be redesigned to
 * maintain the update region.
 */

void XDSetRedrawFlag(void)
{
    redraw = TRUE;
}

/*
 * Function: XDClearDisplay
 * ------------------------
 * This function erases the entire display by filling with the
 * erase color.
 */

void XDClearDisplay(void)
{
    int itemp;
    unsigned int width, height, utemp;
    Window wtemp;

    if (XGetGeometry(disp, osWindow, &wtemp, &itemp, &itemp,
                     &width, &height, &utemp, &utemp) == 0) return;
    XFillRectangle(disp, osWindow, eraseGC, 0, 0, width, height);
}

/*
 * Function: XDDrawLine
 * --------------------
 * This function draws the requested line unless a region is in
 * progress, in which case it adds the line segment to the polygon.
 */

void XDDrawLine(double x, double y, double dx, double dy)
{
    int x0, y0, x1, y1;

    x0 = ScaleX(x);
    y0 = ScaleY(y);
    x1 = ScaleX(x + dx);
    y1 = ScaleY(y + dy);
    if (regionStarted) {
        AddSegment(x0, y0, x1, y1);
    } else {
        XDrawLine(disp, osWindow, (eraseMode) ? eraseGC : drawGC,
                  x0, y0, x1, y1);
    }
}

/*
 * Function: XDDrawArc
 * -------------------
 * This function ordinarily scales its arguments and uses them
 * to draw an arc using the standard XDrawArc call.  If, however,
 * a region has been started, that arc must be rendered using
 * line segments, which is handled by RenderArc.
 */

void XDDrawArc(double x, double y, double rx, double ry,
                double start, double sweep)
{
    int ixc, iyc, irx, iry, istart, isweep;

    if (regionStarted) {
        RenderArc(x, y, rx, ry, start, sweep);
    } else {
        ixc = ScaleX(x);
        iyc = ScaleY(y);
        irx = PixelsX(rx);
        iry = PixelsY(ry);
        istart = GLRound(start);
        isweep = GLRound(sweep);
        if (isweep < 0) {
            isweep = -isweep;
            istart -= isweep;
        }
        if (istart < 0) {
            istart = 360 - (-istart % 360);
        }
        istart %= 360;
        XDrawArc(disp, osWindow, (eraseMode) ? eraseGC : drawGC,
                 ixc - irx, iyc - iry, 2 * irx, 2 * iry,
                 64 * istart, 64 * isweep);
    }
}

/*
 * Function: XDDrawText
 * --------------------
 * This function transforms the client arguments and makes the
 * appropriate X call to display text on the screen.
 */

void XDDrawText(double x, double y, string text)
{
    int ix, iy;

    ix = ScaleX(x);
    iy = ScaleY(y);
    XDrawString(disp, osWindow, (eraseMode) ? eraseGC : drawGC,
                ix, iy, text, strlen(text));
}

/*
 * Function: XDTextWidth
 * ---------------------
 * This function simply calls the XTextWidth function to compute the
 * width of the displayed text and scales it to the client coordinate
 * system.
 */

double XDTextWidth(string text)
{
    return (InchesX(XTextWidth(fontInfo, text, strlen(text))));
}

/*
 * Function: XDSetFont
 * -------------------
 * This function searches the font names table for a font that matches
 * the argument characteristics as closely as possible.  As required
 * by the extgraph.h client interface, the function does not change
 * the font if no such font exists and selects the closest existing
 * size.
 */

string XDSetFont(string font, int size, int style)
{
    char fontbuf[MaxFontName + 30];
    string fontName, *fontList;
    int i, nFonts, bestIndex, bestSize, thisSize;
    bool ok;

    fontName = ConvertToLowerCase(font);
    if (StringEqual(fontName, "default")) {
        sprintf(fontbuf, "*-%s-*", DefaultFont);
    } else {
        sprintf(fontbuf, "*-%s-*", fontName);
    }
    FreeBlock(fontName);
    fontList = XListFonts(disp, fontbuf, MaxFontList, &nFonts);
    if (nFonts != 0) {
        bestIndex = -1;
        for (i = 1; i < nFonts; i++) {
            fontName = fontList[i];
            ok = TRUE;
            if (style & Bold) {
                ok &= FindString("-bold-", fontName, 0) != -1;
            } else {
                ok &= FindString("-medium-", fontName, 0) != -1;
            }
            if (style & Italic) {
                ok &= FindString("-i-", fontName, 0) != -1;
            } else {
                ok &= FindString("-r-", fontName, 0) != -1;
            }
            if (ok) {
                thisSize = SizeFromFontName(fontName);
                if (bestIndex == -1 || abs(size - thisSize)
                                        < abs(size - bestSize)) {
                    bestSize = thisSize;
                    bestIndex = i;
                }
            }
        }
        if (bestIndex != -1) {
            if (fontInfo != NULL) XFreeFont(disp, fontInfo);
            fontInfo = XLoadQueryFont(disp, fontList[bestIndex]);
            if (fontInfo == NULL) {
                Error("Internal error: Can't find font");
            }
            XSetFont(disp, drawGC, fontInfo->fid);
            XSetFont(disp, eraseGC, fontInfo->fid);
            XFreeFontNames(fontList);
            currentFont = CopyString(font);
            currentSize = bestSize;
            currentStyle = style;
        }
    }
    sprintf(fontbuf, "%d %d %s", currentSize, currentStyle, currentFont);
    return (CopyString(fontbuf));
}

/*
 * Function: DisplayFontMetrics
 * ----------------------------
 * This function returns the necessary font metric information through
 * its argument pointers.
 */

void DisplayFontMetrics(double *pAscent, double *pDescent, double *pHeight)
{
    *pAscent = InchesY(fontInfo->max_bounds.ascent);
    *pDescent = InchesY(fontInfo->max_bounds.descent);
    *pHeight = InchesY(fontInfo->ascent + fontInfo->descent);
}

/*
 * Function: XDSetTitle
 * --------------------
 * This function copies the user-supplied string into the window and
 * icon name.
 */

void XDSetTitle(string title)
{
    XTextProperty tp;

    XStringListToTextProperty(&title, 1, &tp);
    XSetWMIconName(disp, mainWindow, &tp);
    XSetWMName(disp, mainWindow, &tp);
}

/*
 * Function: XDSetEraseMode
 * ------------------------
 * This function sets the internal state of the display logic so
 * that it maintains the correct state of the eraseMode flag.  In
 * the rest of the code, the eraseMode flag is used to control
 * which colors are used.
 */

void XDSetEraseMode(bool flag)
{
    eraseMode = flag;
}

/*
 * Function: XDStartRegion
 * -----------------------
 * This function changes the state of the package so that subsequent
 * calls to XDDrawLine and XDDrawArc are used to add segments to a
 * polygonal region instead of having them appear on the display.
 * See the code for StartPolygon, AddSegment, and DisplayPolygon for
 * details.
 */

void XDStartRegion(double grayScale)
{
    regionStarted = TRUE;
    regionGrayScale = grayScale;
    StartPolygon();
}

/*
 * Function: XDEndRegion
 * ---------------------
 * This function closes the region opened by XDStartRegion
 * and displays the assembled polygon.
 */

void XDEndRegion(void)
{
    DisplayPolygon();
    regionStarted = FALSE;
}

/*
 * Function: XDGetMouse
 * --------------------
 * This function returns the current mouse state through the
 * argument pointers.
 */

void XDGetMouse(bool *buttonStateP, double *xp, double *yp)
{
    Window root, child;
    int rootX, rootY, winX, winY;
    unsigned int buttons;

    (void) XQueryPointer(disp, mainWindow, &root, &child,
                         &rootX, &rootY, &winX, &winY, &buttons);
    *buttonStateP = (buttons != 0);
    *xp = InchesX(winX);
    *yp = windowHeight - InchesY(winY);
}

/*
 * Function: XDWaitForMouse
 * ------------------------
 * This function waits for the mouse to enter the state given
 * by down.
 */

void XDWaitForMouse(bool buttonState)
{
    bool currentState;
    double x, y;

    XDGetMouse(&currentState, &x, &y);
    if (buttonState == currentState) {
        XMReleaseClient();
    } else {
        waitState = (buttonState) ? WaitingForMouseDown : WaitingForMouseUp;
    }
}

/*
 * Function: XDSetColor
 * --------------------
 * This function sets the pen color as specified by the arguments.
 */

void XDSetColor(double red, double green, double blue)
{
    XColor color;

    color.red = red * 65535;
    color.green = green * 65535;
    color.blue = blue * 65535;
    if (XAllocColor(disp, colormap, &color) != 0) {
        drawColor = color.pixel;
        XSetForeground(disp, drawGC, drawColor);
    }
}

/*
 * Function: XDSetWindowSize
 * -------------------------
 * This function sets the width and height values of the window.
 */

void XDSetWindowSize(double width, double height)
{
    windowWidth = width;
    windowHeight = height;
}

/*
 * Function: XDGetScreenSize
 * -------------------------
 * This function returns the screen size and may be called
 * prior to the XDOpenDisplay call or from the client fork.
 */

void XDGetScreenSize(double *pScreenWidth, double *pScreenHeight)
{
    StartToOpenDisplay();
    *pScreenWidth = screenWidth;
    *pScreenHeight = screenHeight;
}

/*
 * Function: XDGetResolution
 * -------------------------
 * This function returns the screen resolution, possibly modified by
 * the scale reduction.
 */

void XDGetResolution(double *pXDPI, double *pYDPI)
{
    StartToOpenDisplay();
    *pXDPI = xdpi;
    *pYDPI = ydpi;
}

/*
 * Function: XDGetNColors
 * ----------------------
 * This function returns the number of colors supported by the screen.
 */

int XDGetNColors(void)
{
    StartToOpenDisplay();
    return (1 << pixelDepth);
}

/* Private functions */

/*
 * Function: StartToOpenDisplay
 * Usage: StartToOpenDisplay();
 * ----------------------------
 * This function checks to see if the display is open and, if
 * not, does just enough work to ensure that the fixed parameters
 * are set.
 */

static void StartToOpenDisplay(void)
{
    int scnum;
    Screen *screen;

    if (displayIsOpen) return;
    if ((disp = XOpenDisplay(NULL)) == NULL) {
        Error("Can't open display");
    }
    scnum = DefaultScreen(disp);
    screen = ScreenOfDisplay(disp, scnum);
    pixelDepth = DefaultDepth(disp, scnum);
    screenWidth = WidthMMOfScreen(screen) / 25.4;
    screenHeight = HeightMMOfScreen(screen) / 25.4;
    xdpi = WidthOfScreen(screen) / screenWidth;
    ydpi = HeightOfScreen(screen) / screenHeight;
    displayIsOpen = TRUE;
}

/*
 * Function: InitGC
 * Usage: InitGC();
 * ----------------
 * This function is used as part of the initialization and creates
 * all of the GC structures used for drawing.
 */

static void InitGC(void)
{
    XGCValues gcv;
    Window root;
    int i;

    gcv.foreground = eraseColor;
    gcv.background = eraseColor;
    eraseGC = XCreateGC(disp, osWindow, GCFgBg, &gcv);
    gcv.foreground = drawColor;
    mainGC = XCreateGC(disp, mainWindow, GCFgBg, &gcv);
    drawGC = XCreateGC(disp, osWindow, GCFgBg, &gcv);
    gcv.fill_style = FillOpaqueStippled;
    root = DefaultRootWindow(disp);
    for (i = 0; i < NGrays; i++) {
        grayStipple[i] = gcv.stipple =
           XCreateBitmapFromData(disp, root, grayList[i], 8, 8);
        grayGC[i] = XCreateGC(disp, osWindow,
                              GCFgBg | GCStipple | GCFillStyle, &gcv);
    }
}

/*
 * Function: ForceRedraw
 * Usage: ForceRedraw();
 * ---------------------
 * This function forces the screen to update itself by sending an
 * Expose event for the window.
 */

static void ForceRedraw(void)
{
    XEvent event;

    event.type = Expose;
    event.xany.window = mainWindow;
    event.xexpose.count = 0;
    XSendEvent(disp, mainWindow, False, 0, &event);
    XFlush(disp);
}

/*
 * Function: RedrawWindow
 * Usage: RedrawWindow();
 * ----------------------
 * This function redraws the active display window by copying bits
 * from the offscreen bitmap.
 */

static void RedrawWindow(void)
{
    int itemp;
    unsigned int width, height, utemp;
    Window wtemp;

    if (XGetGeometry(disp, mainWindow, &wtemp, &itemp, &itemp,
                     &width, &height, &utemp, &utemp) == 0) return;
    XCopyArea(disp, osWindow, mainWindow, mainGC, 0, 0, width, height,
              0, 0);
    redraw = FALSE;
}

/*
 * Functions: StartPolygon, AddSegment, EndPolygon
 * Usage: StartPolygon();
 *        AddSegment(x0, y0, x1, y1);
 *        AddSegment(x1, y1, x2, y2);
 *        . . .
 *        DisplayPolygon();
 * -----------------------------------------------
 * These functions implement the notion of a region in the X
 * world, where the easiest shape to fill is a polygon.  Calling
 * StartPolygon initializes the array polygonPoints so that
 * subsequent calls to AddSegment will add points to it.
 * The points in the polygon are assumed to be contiguous,
 * because the client interface checks for this property.
 * Because polygons involving arcs can be quite large, the
 * AddSegment code extends the polygonPoints list if needed
 * by doubling the size of the array.  All storage is freed
 * after calling DisplayPolygon, which uses the XFillPolygon
 * call to generate the display.
 */

static void StartPolygon(void)
{
    polygonPoints = NewArray(PStartSize, XPoint);
    polygonSize = PStartSize;
    nPolygonPoints = 0;
}

static void AddSegment(int x0, int y0, int x1, int y1)
{
    XPoint *newPolygon;
    int i;

    if (nPolygonPoints >= polygonSize) {
        polygonSize *= 2;
        newPolygon = NewArray(polygonSize, XPoint);
        for (i = 0; i < nPolygonPoints; i++) {
            newPolygon[i] = polygonPoints[i];
        }
        FreeBlock(polygonPoints);
        polygonPoints = newPolygon;
    }
    if (nPolygonPoints == 0) {
        polygonPoints[nPolygonPoints].x = x0;
        polygonPoints[nPolygonPoints].y = y0;
        nPolygonPoints++;
    }
    polygonPoints[nPolygonPoints].x = x1;
    polygonPoints[nPolygonPoints].y = y1;
    nPolygonPoints++;
}

static void DisplayPolygon(void)
{
    GC fillGC;
    int i, px;

    if (eraseMode) {
        fillGC = eraseGC;
    } else {
        px = regionGrayScale * (NGrays - 1) + 0.5 - Epsilon;
        fillGC = grayGC[px];
        XSetForeground(disp, fillGC, drawColor);
    }
    if (polygonPoints[0].x != polygonPoints[nPolygonPoints-1].x
        || polygonPoints[0].y != polygonPoints[nPolygonPoints-1].y) {
        polygonPoints[nPolygonPoints++] = polygonPoints[0];
    }
    XFillPolygon(disp, osWindow, fillGC,
                 polygonPoints, nPolygonPoints,
                 Complex, CoordModeOrigin);
    FreeBlock(polygonPoints);
}

/*
 * Function: RenderArc
 * Usage: RenderArc(x, y, rx, ry, start, sweep);
 * ---------------------------------------------
 * This function is identical to the XDDrawArc function except
 * that the arc is rendered using line segments as part of a
 * polygonal region.
 */

static void RenderArc(double x, double y, double rx, double ry,
                      double start, double sweep)
{
    double t, mint, maxt, dt;
    int ix0, iy0, ix1, iy1;

    if (sweep < 0) {
        start += sweep;
        sweep = -sweep;
    }
    dt = atan2(InchesY(1), GLMaxF(fabs(rx), fabs(ry)));
    mint = GLRadians(start);
    maxt = GLRadians(start + sweep);
    ix0 = ScaleX(x + rx * cos(mint));
    iy0 = ScaleY(y + ry * sin(mint));
    for (t = mint + dt; t < maxt; t += dt) {
        if (t > maxt - dt / 2) t = maxt;
        ix1 = ScaleX(x + rx * cos(t));
        iy1 = ScaleY(y + ry * sin(t));
        AddSegment(ix0, iy0, ix1, iy1);
        ix0 = ix1;
        iy0 = iy1;
    }
}

/*
 * Function: SizeFromFontName
 * Usage: SizeFromFontName();
 * --------------------------
 * This function is a simple utility for XDSetFont that returns
 * the font size from an X font name.
 */

static int SizeFromFontName(string fontName)
{
    char *sizePtr;

    sizePtr = strstr(fontName, "--");
    if (sizePtr == NULL) return (-1);
    return (atoi(sizePtr + 2));
}

/* Low-level conversion functions */

/*
 * Functions: InchesX, InchesY
 * Usage: inches = InchesX(pixels);
 *        inches = InchesY(pixels);
 * --------------------------------
 * These functions convert distances measured in pixels to inches.
 * Because the resolution may not be uniform in the horizontal and
 * vertical directions, the coordinates are treated separately.
 */

static double InchesX(int x)
{
    return ((double) x / xdpi);
}

static double InchesY(int y)
{
    return ((double) y / ydpi);
}

/*
 * Functions: PixelsX, PixelsY
 * Usage: pixels = PixelsX(inches);
 *        pixels = PixelsY(inches);
 * --------------------------------
 * These functions convert distances measured in inches to pixels.
 */

static int PixelsX(double x)
{
    return (GLRound(x * xdpi + Epsilon));
}

static int PixelsY(double y)
{
    return (GLRound(y * ydpi + Epsilon));
}

/*
 * Functions: ScaleX, ScaleY
 * Usage: pixels = ScaleX(inches);
 *        pixels = ScaleY(inches);
 * -------------------------------
 * These functions are like PixelsX and PixelsY but convert coordinates
 * rather than lengths.  The difference is that y-coordinate values must
 * be inverted top to bottom to support the cartesian coordinates of
 * the graphics.h model.
 */

static int ScaleX(double x)
{
    return (PixelsX(x));
}

static int ScaleY(double y)
{
    return (PixelsY(windowHeight - y));
}
