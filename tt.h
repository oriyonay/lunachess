// tt.h: struct for a single transposition table entry
#ifndef TT_H
#define TT_H

#include "consts.h"
#include "globals.h"

struct tt_entry {
  // the hash of the position:
  U64 hash;

  // the move value:
  int value;

  // the best move found in this position:
  int best_move;

  // the depth to which the engine analyzed this position:
  char depth;

  // the transposition table flag (either TT_EXACT, TT_ALPHA, or TT_BETA):
  char flag;

  // the age of the transposition table at the time of insertion:
  char tt_age;

  // is this entry a PV node?
  bool is_pv;
};

struct transposition_table {
  // the transposition table entries:
  tt_entry* TT;

  // the age of the transposition table:
  char age;

  // number of entries & size in bytes:
  U64 num_entries;
  U64 size;

  // the mask we use for TT indexing:
  U64 mask;

  // the constructor & destructor:
  transposition_table(U64 MB);
  ~transposition_table();

  // probe(): probe the transposition table for the given position:
  int probe(int depth, int alpha, int beta);

  // put(): write a new entry to the transposition table:
  void put(int depth, int value, int best_move, char flag, bool is_pv);

  // clear(): clears the table:
  void clear();
};

#endif
