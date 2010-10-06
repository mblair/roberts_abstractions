/*
 * File: graphics.c
 * Version: 3.0
 * Last modified on Sat Oct  1 12:28:01 1994 by eroberts
 * -----------------------------------------------------
 * This file is the top-level file in the implementation of the
 * graphics.h interface for X windows.  The complete implementation
 * also includes the following subsidiary modules:
 *
 *   glibrary.h  Various low-level mathematical functions
 *   xmanager.h  Mediates communication with the X operations
 *   xdisplay.h  Performs the actual drawing operations
 *   xcompat.h   Maintains BSD compatibility on System V.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>

#include "genlib.h"
#include "gcalloc.h"
#include "simpio.h"
#include "strlib.h"
#include "extgraph.h"
#include "glibrary.h"
#include "xmanager.h"
#include "xdisplay.h"
#include "xcompat.h"

/*
 * Parameters
 * ----------
 * DesiredWidth  -- Desired width of the graphics window in inches
 * DesiredHeight -- Desired height of the graphics window in inches
 * DefaultSize   -- Default point size
 * MaxColors     -- Maximum number of color names allowed
 * MinColors     -- Minimum number of colors the device must support
 */

#define DesiredWidth    7.0
#define DesiredHeight   4.0
#define DefaultSize    12
#define MaxColors     256
#define MinColors      16

/*
 * Type: graphicsStateT
 * --------------------
 * This structure holds the variables that make up the graphics state.
 */

typedef struct graphicsStateT {
    double cx, cy;
    string font;
    int size;
    int style;
    bool erase;
    int color;
    struct graphicsStateT *link;
} *graphicsStateT;

/*
 * Type: regionStateT
 * ------------------
 * The region assembly process has the character of a finite state
 * machine with the following four states:
 *
 *   NoRegion       Region has not yet been started
 *   RegionStarting Region is started but no line segments yet
 *   RegionActive   First line segment appears
 *   PenHasMoved    Pen has moved during definition
 *
 * The current state determines whether other operations are legal
 * at that point.
 */

typedef enum {
    NoRegion, RegionStarting, RegionActive, PenHasMoved
} regionStateT;

/*
 * Type: colorEntryT
 * -----------------
 * This type is used for the entries in the color table.
 */

typedef struct {
    string name;
    double red, green, blue;
} colorEntryT;

/*
 * Global variables
 * ----------------
 * initialized   -- TRUE if initialization has been done
 * windowTitle   -- Current window title (initialized statically)
 * cmdBuffer     -- Static buffer for sending commands
 * regionState   -- Current state of the region
 * colorTable    -- Table of defined colors
 * nColors       -- Number of defined colors
 * colorOK       -- TRUE if the display supports color
 * lastColor     -- Previous color to avoid multiple changes
 * fontChanged   -- TRUE if font information has changed
 * windowWidth   -- Width of the window in inches
 * windowHeight  -- Height of the window in inches
 * stateStack    -- Stack of graphicStateT blocks
 * cx, cy        -- Current coordinates     | These
 * eraseMode     -- Setting of erase flag   | variables
 * textFont      -- Current font            | consititute
 * textStyle     -- Current style           | the current
 * pointSize     -- Current point size      | graphics
 * penColor      -- Color of pen            | state
 */

static bool initialized = FALSE;
static string windowTitle = "Graphics Window";

static char cmdBuffer[CommandBufferSize];

static regionStateT regionState;

static colorEntryT colorTable[MaxColors];
static int nColors;
static bool colorOK;
static int lastColor;
static bool fontChanged;

static double windowWidth = DesiredWidth;
static double windowHeight = DesiredHeight;

static graphicsStateT stateStack;

static double cx, cy;
static bool eraseMode;
static string textFont;
static int textStyle;
static int pointSize;
static int penColor;

/* Private function prototypes */

static void InitCheck(void);
static void InitGraphicsState(void);
static void InstallFont(void);
static void InitColors(void);
static int FindColorName(string name);
static bool ShouldBeWhite(void);
static bool StringMatch(string s1, string s2);
static void USleep(unsigned useconds);

/* Exported entries */

/* Section 1 -- Basic functions from graphics.h */

/*
 * Function: InitGraphics
 * ----------------------
 * The implementation below hides considerable complexity underneath
 * the InitXHandler call.  If you are trying to modify or maintain
 * this implementation, it is important to understand how that
 * function is implemented.  For details, see the xhandler.c
 * implementation.
 */

void InitGraphics(void)
{
    if (initialized) {
        XMSendCommand(ClearCmd, "");
    } else {
        initialized = TRUE;
        ProtectVariable(stateStack);
        ProtectVariable(windowTitle);
        ProtectVariable(textFont);
        XDSetWindowSize(windowWidth, windowHeight);
        XMInitialize(windowTitle);
        InitColors();
    }
    InitGraphicsState();
}

void MovePen(double x, double y)
{
    InitCheck();
    if (regionState == RegionActive) regionState = PenHasMoved;
    cx = x;
    cy = y;
}

void DrawLine(double dx, double dy)
{
    InitCheck();
    switch (regionState) {
      case NoRegion: case RegionActive:
        break;
      case RegionStarting:
        regionState = RegionActive;
        break;
      case PenHasMoved:
        Error("Region segments must be contiguous");
    }
    sprintf(cmdBuffer, "%.12g %.12g %.12g %.12g", cx, cy, dx, dy);
    XMSendCommand(LineCmd, cmdBuffer);
    cx += dx;
    cy += dy;
}

void DrawArc(double r, double start, double sweep)
{
    DrawEllipticalArc(r, r, start, sweep);
}

double GetWindowWidth(void)
{
    return (windowWidth);
}

double GetWindowHeight(void)
{
    return (windowHeight);
}

double GetCurrentX(void)
{
    InitCheck();
    return (cx);
}

double GetCurrentY(void)
{
    InitCheck();
    return (cy);
}

/* Section 2 -- Elliptical arcs */

void DrawEllipticalArc(double rx, double ry,
                       double start, double sweep)
{
    double x, y;

    InitCheck();
    switch (regionState) {
      case NoRegion: case RegionActive:
        break;
      case RegionStarting:
        regionState = RegionActive;
        break;
      case PenHasMoved:
        Error("Region segments must be contiguous");
    }
    x = cx + rx * cos(GLRadians(start + 180));
    y = cy + ry * sin(GLRadians(start + 180));
    sprintf(cmdBuffer, "%.12g %.12g %.12g %.12g %.12g %.12g",
                       x, y, rx, ry, start, sweep);
    XMSendCommand(ArcCmd, cmdBuffer);
    cx = x + rx * cos(GLRadians(start + sweep));
    cy = y + ry * sin(GLRadians(start + sweep));
}

/* Section 3 -- Graphical structures */

void StartFilledRegion(double density)
{
    InitCheck();
    if (regionState != NoRegion) {
        Error("Region is already in progress");
    }
    if (density < 0 || density > 1) {
        Error("Density for regions must be between 0 and 1");
    }
    regionState = RegionStarting;
    sprintf(cmdBuffer, "%.12g", density);
    XMSendCommand(StartRegionCmd, cmdBuffer);
}

void EndFilledRegion(void)
{
    InitCheck();
    if (regionState == NoRegion) {
        Error("EndFilledRegion without StartFilledRegion");
    }
    regionState = NoRegion;
    XMSendCommand(EndRegionCmd, "");
}

/* Section 4 -- String functions */

void DrawTextString(string text)
{
    InitCheck();
    if (regionState != NoRegion) {
        Error("Text strings are illegal inside a region");
    }
    if (strlen(text) > MaxTextString) {
        Error("Text string too long");
    }
    InstallFont();
    sprintf(cmdBuffer, "%.12g %.12g %s", cx, cy, text);
    XMSendCommand(TextCmd, cmdBuffer);
    cx += TextStringWidth(text);
}

double TextStringWidth(string text)
{
    double result;

    InitCheck();
    if (strlen(text) > MaxTextString) {
        Error("Text string too long");
    }
    InstallFont();
    sprintf(cmdBuffer, "%s", text);
    XMSendCommand(WidthCmd, cmdBuffer);
    XMGetResponse(cmdBuffer);
    (void) sscanf(cmdBuffer, "%lg", &result);
    return (result);
}

void SetFont(string font)
{
    InitCheck();
    if (strlen(font) > MaxFontName) Error("Font name too long");
    textFont = CopyString(font);
    fontChanged = TRUE;
}

string GetFont(void)
{
    InitCheck();
    InstallFont();
    return (CopyString(textFont));
}

void SetPointSize(int size)
{
    InitCheck();
    pointSize = size;
    fontChanged = TRUE;
}

int GetPointSize(void)
{
    InitCheck();
    InstallFont();
    return (pointSize);
}

void SetStyle(int style)
{
    InitCheck();
    textStyle = style;
    fontChanged = TRUE;
}

int GetStyle(void)
{
    InitCheck();
    InstallFont();
    return (textStyle);
}

double GetFontAscent(void)
{
    double ascent;

    InitCheck();
    InstallFont();
    XMSendCommand(FontMetricsCmd, "");
    XMGetResponse(cmdBuffer);
    (void) sscanf(cmdBuffer, "%lg", &ascent);
    return (ascent);
}

double GetFontDescent(void)
{
    double descent;

    InitCheck();
    InstallFont();
    XMSendCommand(FontMetricsCmd, "");
    XMGetResponse(cmdBuffer);
    (void) sscanf(cmdBuffer, "%*lg %lg", &descent);
    return (descent);
}

double GetFontHeight(void)
{
    double height;

    InitCheck();
    InstallFont();
    XMSendCommand(FontMetricsCmd, "");
    XMGetResponse(cmdBuffer);
    (void) sscanf(cmdBuffer, "%*lg %*lg %lg", &height);
    return (height);
}

/* Section 5 -- Mouse support */

double GetMouseX(void)
{
    double x, y;
    int state;

    InitCheck();
    XMSendCommand(GetMouseCmd, "");
    XMGetResponse(cmdBuffer);
    (void) sscanf(cmdBuffer, "%d, %lg, %lg", &state, &x, &y);
    return (x);
}

double GetMouseY(void)
{
    string line;
    double x, y;
    int state;

    InitCheck();
    XMSendCommand(GetMouseCmd, "");
    XMGetResponse(cmdBuffer);
    (void) sscanf(cmdBuffer, "%d, %lg, %lg", &state, &x, &y);
    return (y);
}

bool MouseButtonIsDown(void)
{
    string line;
    double x, y;
    int state;

    InitCheck();
    XMSendCommand(GetMouseCmd, "");
    XMGetResponse(cmdBuffer);
    (void) sscanf(cmdBuffer, "%d, %lg, %lg", &state, &x, &y);
    return (state != 0);
}

void WaitForMouseDown(void)
{
    InitCheck();
    XMSendCommand(WaitForMouseCmd, "D");
    XMGetResponse(cmdBuffer);
}

void WaitForMouseUp(void)
{
    InitCheck();
    XMSendCommand(WaitForMouseCmd, "U");
    XMGetResponse(cmdBuffer);
}

/* Section 6 -- Color support */

bool HasColor(void)
{
    InitCheck();
    return (colorOK);
}

void SetPenColor(string color)
{
    int cindex;

    InitCheck();
    cindex = FindColorName(color);
    if (cindex == -1) Error("Undefined color: %s", color);
    penColor = cindex;
    if (penColor == lastColor) return;
    lastColor = penColor;
    if (HasColor()) {
        sprintf(cmdBuffer, "%g %g %g\n",
                colorTable[cindex].red,
                colorTable[cindex].green,
                colorTable[cindex].blue);
        XMSendCommand(SetColorCmd, cmdBuffer);
    } else {
        SetEraseMode(eraseMode);
    }
}

string GetPenColor(void)
{
    InitCheck();
    return (colorTable[penColor].name);
}

void DefineColor(string name,
                 double red, double green, double blue)
{
    int cindex;

    InitCheck();
    if (red < 0 || red > 1 || green < 0 || green > 1 || blue < 0 || blue > 1) {
        Error("DefineColor: All color intensities must be between 0 and 1");
    }
    cindex = FindColorName(name);
    if (cindex == -1) {
        if (nColors == MaxColors) Error("DefineColor: Too many colors");
        cindex = nColors++;
    }
    colorTable[cindex].name = CopyString(name);
    colorTable[cindex].red = red;
    colorTable[cindex].green = green;
    colorTable[cindex].blue = blue;
}

/* Section 7 -- Miscellaneous functions */

void SetEraseMode(bool mode)
{
    InitCheck();
    eraseMode = mode;
    sprintf(cmdBuffer, "%d", (int) (mode || ShouldBeWhite()));
    XMSendCommand(SetEraseCmd, cmdBuffer);
}

bool GetEraseMode(void)
{
    InitCheck();
    return (eraseMode);
}

void SetWindowTitle(string title)
{
    windowTitle = CopyString(title);
    if (initialized) {
        sprintf(cmdBuffer, "%s", windowTitle);
        XMSendCommand(SetTitleCmd, cmdBuffer);
    }
}

string GetWindowTitle(void)
{
    return (CopyString(windowTitle));
}

void UpdateDisplay(void)
{
    int cnt;

    InitCheck();
    XMSendCommand(UpdateCmd, "");
}

void Pause(double seconds)
{
    if (initialized) UpdateDisplay();
    USleep((unsigned) (seconds * 1000000));
}

void ExitGraphics(void)
{
    XMSendCommand(ExitGraphicsCmd, "");
    exit(0);
}

void SaveGraphicsState(void)
{
    graphicsStateT sb;

    InitCheck();
    sb = New(graphicsStateT);
    sb->cx = cx;
    sb->cy = cy;
    sb->font = textFont;
    sb->size = pointSize;
    sb->style = textStyle;
    sb->erase = eraseMode;
    sb->color = penColor;
    sb->link = stateStack;
    stateStack = sb;
}

void RestoreGraphicsState(void)
{
    graphicsStateT sb;

    InitCheck();
    if (stateStack == NULL) {
        Error("RestoreGraphicsState called before SaveGraphicsState");
    }
    sb = stateStack;
    cx = sb->cx;
    cy = sb->cy;
    textFont = sb->font;
    pointSize = sb->size;
    textStyle = sb->style;
    eraseMode = sb->erase;
    penColor = sb->color;
    stateStack = sb->link;
    FreeBlock(sb);
    fontChanged = TRUE;
    SetEraseMode(eraseMode);
    SetPenColor(colorTable[penColor].name);
}

double GetFullScreenWidth(void)
{
    double screenWidth, screenHeight;

    XDGetScreenSize(&screenWidth, &screenHeight);
    return (screenWidth);
}

double GetFullScreenHeight(void)
{
    double screenWidth, screenHeight;

    XDGetScreenSize(&screenWidth, &screenHeight);
    return (screenHeight);
}

void SetWindowSize(double width, double height)
{
    if (initialized) {
        Error("The window size cannot be set after calling InitGraphics");
    }
    windowWidth = width;
    windowHeight = height;
}

double GetXResolution(void)
{
    double xdpi, ydpi;

    XDGetResolution(&xdpi, &ydpi);
    return (ydpi);
}

double GetYResolution(void)
{
    double xdpi, ydpi;

    XDGetResolution(&xdpi, &ydpi);
    return (ydpi);
}

/* Private functions */

/*
 * Function: InitCheck
 * Usage: InitCheck();
 * -------------------
 * This function merely ensures that the package has been
 * initialized before the client functions are called.
 */

static void InitCheck(void)
{
    if (!initialized) Error("InitGraphics has not been called");
}

/*
 * Function: InitGraphicsState
 * Usage: InitGraphicsState();
 * ---------------------------
 * This function initializes the graphics state elements to
 * their default values.
 */

static void InitGraphicsState(void)
{
    cx = cy = 0;
    eraseMode = FALSE;
    textFont = "Default";
    pointSize = DefaultSize;
    textStyle = Normal;
    stateStack = NULL;
    regionState = NoRegion;
    fontChanged = TRUE;
    SetPenColor("Black");
}

static void InstallFont(void)
{
    char fontbuf[MaxFontName];
    string line;

    if (!fontChanged) return;
    sprintf(cmdBuffer, "%d %d %s", pointSize, textStyle, textFont);
    XMSendCommand(SetFontCmd, cmdBuffer);
    XMGetResponse(cmdBuffer);
    (void) sscanf(cmdBuffer, "%d %d %s", &pointSize, &textStyle, fontbuf);
    textFont = CopyString(fontbuf);
    fontChanged = FALSE;
}

/*
 * Function: InitColors
 * Usage: InitColors();
 * --------------------
 * This function defines the built-in colors.
 */

static void InitColors(void)
{
    colorOK = (XDGetNColors() >= MinColors);
    lastColor = -1;
    nColors = 0;
    DefineColor("Black", 0, 0, 0);
    DefineColor("Dark Gray", .35, .35, .35);
    DefineColor("Gray", .6, .6, .6);
    DefineColor("Light Gray", .75, .75, .75);
    DefineColor("White", 1, 1, 1);
    DefineColor("Red", 1, 0, 0);
    DefineColor("Yellow", 1, 1, 0);
    DefineColor("Green", 0, 1, 0);
    DefineColor("Cyan", 0, 1, 1);
    DefineColor("Blue", 0, 0, 1);
    DefineColor("Magenta", 1, 0, 1);
}

/*
 * Function: FindColorName
 * Usage: index = FindColorName(name);
 * -----------------------------------
 * This function returns the index of the named color in the
 * color table, or -1 if the color does not exist.
 */

static int FindColorName(string name)
{
    int i;

    for (i = 0; i < nColors; i++) {
        if (StringMatch(name, colorTable[i].name)) return (i);
    }
    return (-1);
}

static bool ShouldBeWhite(void)
{
    if (colorTable[penColor].red < .9) return (FALSE);
    if (colorTable[penColor].blue < .9) return (FALSE);
    if (colorTable[penColor].green < .9) return (FALSE);
    return (TRUE);
}

/*
 * Function: StringMatch
 * Usage: if (StringMatch(s1, s2)) . . .
 * -------------------------------------
 * This function returns TRUE if two strings are equal, ignoring
 * case distinctions.
 */

static bool StringMatch(string s1, string s2)
{
    register char *cp1, *cp2;

    cp1 = s1;
    cp2 = s2;
    while (tolower(*cp1) == tolower(*cp2)) {
        if (*cp1 == '\0') return (TRUE);
        cp1++;
        cp2++;
    }
    return (FALSE);
}

/*
 * Function: USleep
 * Usage: USleep(useconds);
 * ------------------------
 * This function sleeps for the indicated number of microseconds.
 * Some versions of Unix implement a usleep call, but it does not
 * appear to be standard.  The easiest way to implement it is by
 * calling select with no descriptors, since the compatibility
 * library has already made sure that select is available.
 */

static void USleep(unsigned useconds)
{
    struct timeval tv;

    tv.tv_sec = useconds / 1000000;
    tv.tv_usec = useconds % 1000000;
    (void) select(1, NULL, NULL, NULL, &tv);
}
