#include "tt.h"

// probe_tt(): probe the transposition table for the given position:
int probe_tt(int depth, int alpha, int beta) {
  tt_entry* entry = &TT[b.hash % NUM_TT_ENTRIES];

  // verify that the entry is not a collision:
  if (entry->hash != b.hash) return TT_NO_MATCH;

  // make sure we're at a good enough depth to use the data in this entry:
  if (entry->depth >= depth) {
    if (entry->flag == TT_EXACT) return entry->value;
    if (entry->flag == TT_ALPHA && entry->value <= alpha) return alpha;
    if (entry->flag == TT_BETA && entry->value >= beta) return beta;
  }

  // should never be called, but here just in case:
  return TT_NO_MATCH;
}

// update_tt(): write a new entry to the transposition table:
void update_tt(int depth, int value, char flag) {
  tt_entry* entry = &TT[b.hash % NUM_TT_ENTRIES];

  // write the data:
  if (depth >= entry->depth) {
    entry->hash = b.hash;
    entry->depth = depth;
    entry->flag = flag;
    entry->value = value;
  }
}
