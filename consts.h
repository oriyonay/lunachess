// CONSTS.H: ALL NON-TUNEABLE CONSTANTS

#ifndef CONSTS_H
#define CONSTS_H

#include <algorithm>
#include <functional>
#include <random>
#include <unordered_map>

#include "board.h"
#include "defs.h"

// initializes all uninitialized constants (bitmasks, etc.)
extern void init_consts();

// returns the RECT_LOOKUP result for a single pair i, j:
extern U64 in_between(int sq1, int sq2);

// calculates rook and bishop masks to be stored in ROOK_MASKS and BISHOP_MASKS:
extern U64 rmask(int sq);
extern U64 bmask(int sq);

// given a magic table hash index, calculate the bitboard of blocking pieces:
extern U64 get_blockers_from_index(int index, U64 mask);

// ENGINE SETTINGS
// extern unsigned int TT_INDEX_MASK; // mask used on hash to get table index

// a 64-bit pseudo-random number generator (for hashing):
extern std::mt19937_64 generator;

// maps piece characters to their index in the board::bitboard array
extern std::unordered_map<char, int> PIECE_INDICES;

// maps board::bitboard indices to their piece character
extern char* PIECE_CHARS;

// for tapered evaluation:
extern const int OPENING_PHASE_SCORE;
extern const int ENDGAME_PHASE_SCORE;

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

// random numbers for zobrist hashing:
extern U64 ZOBRIST_SQUARE_KEYS[12][64];
extern U64 ZOBRIST_CASTLE_RIGHTS_KEYS[16];
extern U64 ZOBRIST_EP_KEYS[8];
extern U64 ZOBRIST_TURN_KEY;

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

// constants for differences in PST values for rooks after castling. used to
// slightly optimize updating the base_score with rooks (so instead of doing
// base_score -= PST[ROOK][OLD POSITION] and then
// base_score += PST[ROOK][NEW POSITION]), we just precalculate the sum.
extern int CWK_ROOK_PST_DIFFERENCE_OPENING;
extern int CWQ_ROOK_PST_DIFFERENCE_OPENING;
extern int CBK_ROOK_PST_DIFFERENCE_OPENING;
extern int CBQ_ROOK_PST_DIFFERENCE_OPENING;

extern int CWK_ROOK_PST_DIFFERENCE_ENDGAME;
extern int CWQ_ROOK_PST_DIFFERENCE_ENDGAME;
extern int CBK_ROOK_PST_DIFFERENCE_ENDGAME;
extern int CBQ_ROOK_PST_DIFFERENCE_ENDGAME;

// precalculation of zobrist keys of rook positions before and after castling:
extern U64 CWK_ROOK_ZOBRIST;
extern U64 CWQ_ROOK_ZOBRIST;
extern U64 CBK_ROOK_ZOBRIST;
extern U64 CBQ_ROOK_ZOBRIST;

/* -------------------- CONSTANTS FOR ENGINE EVALUATION -------------------- */
extern int MVV_LVA_SCORE[12][13]; // contains MVV/LVA precalculated scores

extern U64 ISOLATED_MASKS[64];
extern U64 WHITE_PASSED_PAWN_MASKS[64];
extern U64 BLACK_PASSED_PAWN_MASKS[64];

/* -------------------- MISCELLANEOUS CONSTANTS -------------------- */
// miscellaneous pre-calculated constants:
extern U64 SQUARE_FILES[64];
// extern U64 SQUARE_RANKS[64];
// extern char FILE_OF[64];
// extern char RANK_OF[64];

// tablebase flags, *negated* to allow for the EGTB hack
extern const int TB_VALUES[5];

#endif
