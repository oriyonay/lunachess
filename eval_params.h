#ifndef EVAL_CONSTS_H
#define EVAL_CONSTS_H

#include <fstream>
#include <string>

#include "consts.h"
#include "defs.h"

// functions for loading and exporting evaluation parameters:
extern void load_params(const char* FILENAME);
extern void save_params(const char* FILENAME);

// tuneable engine settings:
extern int ALPHA_PRUNING_DEPTH;
extern int ALPHA_PRUNING_MARGIN;
extern int ASPIRATION_WINDOW_VALUE;
extern int BISHOP_PAIR_BONUS;
extern int DEFAULT_UCI_INPUT_BUFFER_SIZE;
extern int DELTA_VALUE;
extern int DOUBLED_PAWN_PENALTY;
extern int FULLY_OPEN_FILE_BONUS;
extern int ISOLATED_PAWN_PENALTY;
extern int KING_SHIELD_BONUS;
extern int LMP_DEPTH;
extern int LMR_FULL_DEPTH_MOVES;
extern int LMR_REDUCTION_LIMIT;
extern int MULTICUT_M; // number of first moves to consider
extern int MULTICUT_C; // number of cutoffs to cause a multi-cut prune
extern int MULTICUT_R; // depth for multi-cut
extern int NULL_MOVE_PRUNING_DEPTH;
extern int SEMI_OPEN_FILE_BONUS;

// just the positive version of PIECE_TO_MATERIAL[0][:] with king & pawn values set to 0,
// for faster game phase score calculation
extern int GAME_PHASE_MATERIAL_SCORE[13];

// maps piece types (enums) to their material value
extern int PIECE_TO_MATERIAL[2][13];

// the piece-square tables
extern int PIECE_SQUARE_TABLE[2][12][64];

extern int PASSED_PAWN_BONUS[9];
extern int RAZOR_MARGIN[10];
extern int SEE_PIECE_VALUES[13];
extern int LMP_ARRAY[MAX_SEARCH_PLY][2]; // indexed [search depth][improving]

#endif
