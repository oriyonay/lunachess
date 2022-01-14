#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
  #include <io.h>
  #include <windows.h>
#else
  #include <sys/ioctl.h>
  #include <sys/select.h>
  #include <sys/time.h>
	#include <sys/types.h>
  #include <unistd.h>
#endif

#include "consts.h"
#include "defs.h"
#include "globals.h"

using namespace std::chrono;

// print a move given the move code. does NOT print a new line!
void print_move(int m);

// gets the time in milliseconds (as an int - we assume no one's
// going to run this for 50 days straight)
int get_time();

// listen to GUI input while in search. OS-dependent:
// (code is from VICE engine by richard allbert)
int input_waiting();

// read GUI input:
// (code is from BBC engine)
void read_input();

// make sure we didn't get any stop or quit command, and that we still have time:
void communicate();

// perft function for 'go perft' command:
long perft(int depth, bool root);

#endif
