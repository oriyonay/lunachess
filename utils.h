#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include "consts.h"
#include "defs.h"

#ifdef WIN32
  #include <Windows.h>
#else
  #include <sys/time.h>
#endif

// print a move given the move code. does NOT print a new line!
void print_move(int m);

// gets the time in milliseconds (as an int - we assume no one's
// going to run this for 50 days straight)
int get_time();

#endif
