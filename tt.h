// tt.h: struct for a single transposition table entry
#ifndef TT_H
#define TT_H

#include "defs.h"

struct tt_entry {
  // the hash of the position:
  U64 hash;

  // the bitboard of occupied pieces in the position. used as an extra
  // measure of confidence that this is NOT a collision:
  // U64 occupied;

  // the best move (and its score) in this position:
  int best_move;
  int move_score;

  // the depth to which we explored the position:
  char depth;
};

#endif
