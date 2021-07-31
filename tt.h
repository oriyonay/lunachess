// tt.h: struct for a single transposition table entry
#ifndef TT_H
#define TT_H

// macro to clear the transposition table
#define CLEAR_TT() (memset(TT, 0, TT_SIZE))

#include "consts.h"
#include "globals.h"

struct tt_entry {
  // the hash of the position:
  U64 hash;

  // the depth to which the engine analyzed this position:
  char depth;

  // the transposition table flag (either TT_EXACT, TT_ALPHA, or TT_BETA):
  char flag;

  // the move value and the best move found:
  int value;
  int best_move;
};

// probe_tt(): probe the transposition table for the given position:
int probe_tt(int depth, int alpha, int beta);

// update_tt(): write a new entry to the transposition table:
void update_tt(int depth, int value, char flag);

#endif
