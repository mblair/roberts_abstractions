/*
 * File: glibrary.h
 * Version: 3.0
 * Last modified on Sat Oct  1 12:28:02 1994 by eroberts
 * -----------------------------------------------------
 * This interface exports several simple, low-level functions that are
 * used in various other parts of the graphics package implementation.
 * Because these functions are otherwise likely to collide with student
 * functions, all external names include a GL prefix.
 */

#ifndef _glibrary_h
#define _glibrary_h

/*
 * Functions: GLRadians, GLDegrees
 * Usage: radians = GLRadians(degrees);
 *        degrees = GLDegrees(radians);
 * ------------------------------------
 * These functions convert back and forth between degrees and radians.
 */

double GLRadians(double degrees);
double GLDegrees(double radians);

/*
 * Function: GLRound
 * Usage: n = GLRound(x);
 * ----------------------
 * This function rounds a double to the nearest integer.
 */

int GLRound(double x);

/*
 * Functions: GLMin, GLMax
 * Usage: min = GLMin(x, y);
 *        max = GLMax(x, y);
 * -------------------------
 * These functions find the minimum and maximum of two integers.
 */

int GLMin(int x, int y);
int GLMax(int x, int y);

/*
 * Functions: GLMinF, GLMaxF
 * Usage: min = GLMinF(x, y);
 *        max = GLMaxF(x, y);
 * --------------------------
 * These functions find the minimum and maximum of two doubles.
 */

double GLMinF(double x, double y);
double GLMaxF(double x, double y);

#endif
