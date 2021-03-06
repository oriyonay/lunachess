#include "tt.h"

// the transposition table constructor:
transposition_table::transposition_table(U64 MB) {
  // make sure num_entries is a power of 2:
  U64 bytes = MB << 20;
  U64 max_entries = bytes / sizeof(tt_entry);

  // make sure max_entries is a power of 2:
  U64 num_entries = max_entries;
  while (num_entries & (num_entries - 1)) {
    POP_LSB(num_entries);
  }

  this->num_entries = num_entries;
  this->size = num_entries * sizeof(tt_entry);
  this->mask = num_entries - 1;
  this->age = 0;

  TT = new tt_entry[num_entries];
}

// probe(): probe the transposition table for the given position:
int transposition_table::probe(int depth, int alpha, int beta) {
  tt_entry* entry = &TT[b.hash & mask];

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

// put(): write a new entry to the transposition table:
void transposition_table::put(int depth, int value, int best_move, char flag, bool is_pv) {
  tt_entry* entry = &TT[b.hash & mask];

  // write the data:
  if (entry->hash == 0L ||
      entry->tt_age != age ||
      is_pv ||
      (!entry->is_pv && depth >= entry->depth) ||
      (entry->hash == b.hash && depth >= entry->depth))
  {
    entry->hash = b.hash;
    entry->depth = depth;
    entry->flag = flag;
    entry->tt_age = age;
    entry->value = value;
    entry->best_move = best_move;
    entry->is_pv = is_pv;
  }
}

// clear(): clears the table:
void transposition_table::clear() {
  memset(TT, 0, size);
}
