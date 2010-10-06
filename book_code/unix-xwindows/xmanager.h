/*
 * File: xmanager.h
 * Version: 3.0
 * Last modified on Sat Oct  1 12:25:07 1994 by eroberts
 * -----------------------------------------------------
 * This file is the interface to the xmanager module, which is the
 * portion of the graphics implementation responsible for all of the
 * interaction with X.
 */

#ifndef _xmanager_h
#define _xmanager_h

#include "genlib.h"

/*
 * Constants
 * ---------
 * CommandBufferSize  -- Size of internal command buffer
 * MaxFontName        -- Maximum length of font name
 * MaxTextString      -- Length limit for DrawTextString
 */

#define CommandBufferSize   200
#define MaxFontName          25
#define MaxTextString       120

/*
 * Type: commandT
 * --------------
 * The client side of the graphics implementation and the X side of
 * the implementation communicate by means of commands sent across a
 * communication pipe.  This enumeration type defines the command
 * codes.
 */

typedef enum {
   LineCmd,
   ArcCmd,
   TextCmd,
   WidthCmd,
   FontMetricsCmd,
   SetEraseCmd,
   StartRegionCmd,
   EndRegionCmd,
   ClearCmd,
   UpdateCmd,
   SetFontCmd,
   SetTitleCmd,
   GetWidthCmd,
   GetHeightCmd,
   GetMouseCmd,
   WaitForMouseCmd,
   SetColorCmd,
   ExitGraphicsCmd
} commandT;

/*
 * Function: XMInitialize
 * Usage: XMInitialize(title);
 * ---------------------------
 * This function starts the X manager process.  The title argument is
 * a string to use as the title bar for the new window.
 */

void XMInitialize(string title);

/*
 * Function: XMSendCommand
 * Usage: XMSendCommand(cmd, args);
 * --------------------------------
 * This function sends the specified command to the X manager
 * process along with an argument string.
 */

void XMSendCommand(commandT cmd, string args);

/*
 * Function: XMGetResponse
 * Usage: XMGetResponse(buffer);
 * -----------------------------
 * This function reads a line from the X manager in response to
 * a command message.  The string is stored in the buffer provided
 * by the client, which must be at least CommandBufferSize bytes.
 */

void XMGetResponse(char buffer[]);

/*
 * Function: XMReleaseClient
 * Usage: XMReleaseClient();
 * -------------------------
 * This function is used by the X display code to release a
 * client waiting on an event.
 */

void XMReleaseClient(void);

#endif
