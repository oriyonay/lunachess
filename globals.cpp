#include "globals.h"

board b(FEN_START);
int best_move;
int best_score = -INF;
int nodes_evaluated;

// the principal variation table:
int pv_length[MAX_GAME_MOVES];
int pv_table[MAX_GAME_MOVES][MAX_GAME_MOVES];

// to store killer moves and history moves:
int killer_moves[2][MAX_GAME_MOVES]; // [move_id][ply]
int history_moves[12][64]; // [piece][square]
