#include "globals.h"

// init_globals(): initialize global variables:
void init_globals() {
  b = board(FEN_START);
  tt_entry* TT = new tt_entry[NUM_TT_ENTRIES];
  CLEAR_TT();
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
int pv_length[MAX_GAME_MOVES];
int pv_table[MAX_GAME_MOVES][MAX_GAME_MOVES];

// to store killer moves and history moves:
int killer_moves[2][MAX_GAME_MOVES]; // [move_id][ply]
int history_moves[12][64]; // [piece][square]

/* ---------- TRANSPOSITION TABLE GLOBALS ---------- */

tt_entry* TT;
