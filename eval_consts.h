#ifndef EVAL_CONSTS_H
#define EVAL_CONSTS_H

#include "defs.h"

// tuneable engine settings:
extern const int ALPHA_PRUNING_DEPTH;
extern const int ALPHA_PRUNING_MARGIN;
extern const int ASPIRATION_WINDOW_VALUE;
extern const int BISHOP_PAIR_BONUS;
extern const int DEFAULT_UCI_INPUT_BUFFER_SIZE;
extern const int DELTA_VALUE;
extern const int DOUBLED_PAWN_PENALTY;
extern const int FULLY_OPEN_FILE_BONUS;
extern const int ISOLATED_PAWN_PENALTY;
extern const int KING_SHIELD_BONUS;
extern const int LMP_DEPTH;
extern const int LMR_FULL_DEPTH_MOVES;
extern const int LMR_REDUCTION_LIMIT;
extern const int MULTICUT_M; // number of first moves to consider
extern const int MULTICUT_C; // number of cutoffs to cause a multi-cut prune
extern const int MULTICUT_R; // depth for multi-cut
extern const int NULL_MOVE_PRUNING_DEPTH;
extern const int SEMI_OPEN_FILE_BONUS;

// just the positive version of PIECE_TO_MATERIAL[0][:] with king & pawn values set to 0,
// for faster game phase score calculation
extern const int GAME_PHASE_MATERIAL_SCORE[13];

// maps piece types (enums) to their material value
extern const int PIECE_TO_MATERIAL[2][13];

// the piece-square tables
extern int PIECE_SQUARE_TABLE[2][12][64];

extern const int PASSED_PAWN_BONUS[9];
extern const int RAZOR_MARGIN[10];
extern const int SEE_PIECE_VALUES[13];
extern int LMP_ARRAY[MAX_SEARCH_PLY][2]; // indexed [search depth][improving]

#endif
