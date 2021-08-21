// GLOBALS.H: contains non-constant global variables
#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstring>

#include "board.h"
#include "tt.h"

// forward declaration of the transposition table object:
struct tt_entry;
struct transposition_table;

// init_globals(): initialize global variables:
void init_globals();

/* ---------- SEARCH RELATED GLOBALS ---------- */

extern board b;
extern int nodes_evaluated;

// for following and scoring the PV:
extern bool follow_pv;
extern bool score_pv;

// the principal variation table:
extern int pv_length[MAX_SEARCH_PLY];
extern int pv_table[MAX_SEARCH_PLY][MAX_SEARCH_PLY];

// to store killer moves and history moves:
extern int killer_moves[2][MAX_SEARCH_PLY]; // [move_id][ply]
extern int history_moves[12][64]; // [piece][square]

extern int static_evals[MAX_SEARCH_PLY];

/* ---------- TRANSPOSITION TABLE GLOBALS ---------- */

extern transposition_table TT;

/* ---------- TIME CONTROL RELATED GLOBALS ---------- */

// if this flag is turned on, quit the search as soon as possible
extern bool stop_search;

// did we get the quit command while thinking?
extern bool quit_flag;

// are we using time control?
extern bool time_set;

// UCI's movestogo holder:
extern int moves_to_go;
extern bool moves_to_go_enabled;

// UCI's movetime counter:
extern int move_time;

// extern int time;
extern int time_increment;
extern int start_time;
extern int stop_time;
extern int time_limit;

#endif
