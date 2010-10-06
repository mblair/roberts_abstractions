/*
 * File: glibrary.c
 * Version: 3.0
 * Last modified on Sat Oct  1 12:28:01 1994 by eroberts
 * -----------------------------------------------------
 * This file implements the simple functions in the glibrary.h
 * interface.
 */

#include <stdio.h>
#include <math.h>
#include "genlib.h"

/* Constants */

#define Pi 3.1415926535

/* Exported entries */

double GLRadians(double degrees)
{
    return (degrees * Pi / 180);
}

double GLDegrees(double radians)
{
    return (radians * 180 / Pi);
}

int GLRound(double x)
{
    return ((int) floor(x + 0.5));
}

int GLMin(int x, int y)
{
    return ((x < y) ? x : y);
}

int GLMax(int x, int y)
{
    return ((x > y) ? x : y);
}

double GLMinF(double x, double y)
{
    return ((x < y) ? x : y);
}

double GLMaxF(double x, double y)
{
    return ((x > y) ? x : y);
}
