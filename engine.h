/* ENGINE.H: contains the 'intelligent' part of the program, such as the board
 * evaluation function, the move scoring function, and the negamax search
 * algorithm for finding the best move.
*/

#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "globals.h"
#include "utils.h"

/* ---------- the search functions ---------- */
// score_move(): the move scoring function
inline int score_move(int move, int forward_ply);

// evaluate(): the board evaluation function
inline int evaluate();

// the search algorithms for finding the best move:
void search(int depth);
int negamax(int depth, int alpha, int beta, int forward_ply);
int quiescence(int alpha, int beta, int forward_ply);

// SEE and helper functions:
int see(int move);
int estimated_move_value(int move);

#endif
