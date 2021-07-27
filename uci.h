#ifndef UCI_H
#define UCI_H

#include <cstring>
#include <stdio.h>
#include <stdlib.h> // for atoi

#include "defs.h"
#include "engine.h"
#include "globals.h"
#include "utils.h"

// start UCI communication:
void uci_loop();

// parse 'position' command:
void parse_position(char* command);

// parse 'go' command:
void parse_go(char* command);

#endif
