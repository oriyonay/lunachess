#include "globals.h"

// init_globals(): initialize global variables:
void init_globals() {
  b = board(FEN_START);
  TT = transposition_table(NUM_TT_ENTRIES);
  TT.clear();

  stop_search = false;
  quit_flag = false;
  time_set = false;
}

/* ---------- SEARCH RELATED GLOBALS ---------- */

// the board is initialized here but is re-initialized on main(). it's only
// initialized here because this is the only constructor available.
board b(FEN_START);
int nodes_evaluated;

// for following and scoring the PV:
bool follow_pv;
bool score_pv;

// the principal variation table:
int pv_length[MAX_SEARCH_PLY];
int pv_table[MAX_SEARCH_PLY][MAX_SEARCH_PLY];

// to store killer moves and history moves:
int killer_moves[2][MAX_SEARCH_PLY]; // [move_id][ply]
int history_moves[12][64]; // [piece][square]

int static_evals[MAX_SEARCH_PLY];

/* ---------- TRANSPOSITION TABLE GLOBALS ---------- */

transposition_table TT(0);

/* ---------- TIME CONTROL RELATED GLOBALS ---------- */

// if this flag is turned on, quit the search as soon as possible
bool stop_search;

// did we get the quit command while thinking?
bool quit_flag;

// are we using time control?
bool time_set;

// UCI's movestogo holder:
int moves_to_go;
bool moves_to_go_enabled;

// UCI's movetime counter:
int move_time;

int time_increment;
int start_time;
int stop_time;
int time_limit;
