#ifndef GLOBALS_H
#define GLOBALS_H

// GLOBALS.H: contains non-constant global variables
#include "board.h"

extern board b;
extern int nodes_evaluated;

// for following and scoring the PV:
extern bool follow_pv;
extern bool score_pv;

// the principal variation table:
extern int pv_length[MAX_GAME_MOVES];
extern int pv_table[MAX_GAME_MOVES][MAX_GAME_MOVES];

// to store killer moves and history moves:
extern int killer_moves[2][MAX_GAME_MOVES]; // [move_id][ply]
extern int history_moves[12][64]; // [piece][square]

#endif
