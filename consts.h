#ifndef CONSTS_H
#define CONSTS_H

#include <algorithm>
#include <functional>
#include <unordered_map>

#include "defs.h"

// initializes all uninitialized constants (bitmasks, etc.)
extern void init_consts();

// maps piece characters to their index in the board::bitboard array
extern std::unordered_map<char, int> PIECE_INDICES;

// maps piece names to their material value
extern std::unordered_map<char, int> MATERIAL;

// maps piece types (enums) to their material value
extern const int PIECE_TO_MATERIAL[12];

// maps board::bitboard indices to their piece character
extern char* PIECE_CHARS;

// bitmasks for files and ranks:
extern U64 FILES[8];
extern U64 RANKS[9]; // RANKS[0] is unused for human indexing convenience
extern U64 FILE_AB;
extern U64 FILE_GH;

// bitmasks for bitscanning:
extern const U64 DEBRUIJN;
extern const char DEBRUIJN_INDEX[64];

// diagonal and antidiagonal bitmasks:
extern U64 DIAGONAL_MASKS[15];
extern U64 ANTIDIAGONAL_MASKS[15];

// knight-span and king-span bitmasks:
extern U64 KNIGHT_SPAN;
extern U64 KING_SPAN;

// knight-span and king-span bitmasks for the entire board
// (i.e., KNIGHT_MOVES[x] gives a bitmask of all knight attacks for a knight on
// square x)
extern U64 KNIGHT_MOVES[64];
extern U64 KING_MOVES[64];

// bitmasks for spaces between kings and rooks (to check for possible castling)
// the queenside pieces need a special EMPTY_SPACES mask to also include the
// space 3 squares away from the king, which must be empty but does not have
// to be safe (what a detail!)
extern U64 CWK_SAFE_SPACES;
extern U64 CWQ_SAFE_SPACES;
extern U64 CBK_SAFE_SPACES;
extern U64 CBQ_SAFE_SPACES;

extern U64 CWQ_EMPTY_SPACES;
extern U64 CBQ_EMPTY_SPACES;

#endif
