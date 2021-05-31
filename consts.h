#ifndef CONSTS_H
#define CONSTS_H

#include <algorithm>
#include <functional>
#include <unordered_map>

#include "defs.h"

// initializes all uninitialized constants (bitmasks, etc.)
extern void init_consts();

// returns the RECT_LOOKUP result for a single pair i, j:
extern U64 in_between(int sq1, int sq2);

// calculates rook and bishop masks to be stored in ROOK_MASKS and BISHOP_MASKS:
extern U64 rmask(int sq);
extern U64 bmask(int sq);

// given a magic table hash index, calculate the bitboard of blocking pieces:
U64 get_blockers_from_index(int index, U64 mask);

// maps piece characters to their index in the board::bitboard array
extern std::unordered_map<char, int> PIECE_INDICES;

// maps piece names to their material value
extern std::unordered_map<char, int> MATERIAL;

// maps piece types (enums) to their material value
extern const int PIECE_TO_MATERIAL[13];

// maps board::bitboard indices to their piece character
extern char* PIECE_CHARS;

// bitmasks for files and ranks:
extern U64 FILES[8];
extern U64 RANKS[9]; // RANKS[0] is unused for human indexing convenience
extern U64 FILE_AB;
extern U64 FILE_GH;

/*  ---------- TABLES FOR MAGIC BITBOARDS: ---------- */
// masks in the line and diagonal directions for each square. used for masking
// out irrelevant bits from blocker set:
extern U64 ROOK_MASKS[64];
extern U64 BISHOP_MASKS[64];

// magic numbers for rooks and bishops:
extern const U64 ROOK_MAGICS[64];
extern const U64 BISHOP_MAGICS[64];

// number of index bits for each square's hash table:
extern const int ROOK_INDEX_BITS[64];
extern const int BISHOP_INDEX_BITS[64];

// and finally, the actual magic tables for rook and bishop-like pieces:
extern U64 ROOK_TABLE[64][4096];
extern U64 BISHOP_TABLE[64][512];

/*  ---------- END OF MAGIC BITBOARD-RELATED CONSTANTS ---------- */

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

// rectangular lookup matrix: initialized as empty for any 2 squares i, j that
// are not on the same line, and with 1s on the line in-between squares i, j
// otherwise
extern U64 RECT_LOOKUP[64][64];

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
