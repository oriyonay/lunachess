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
// init_search(): initialize and zero-out search structures
inline void init_search();

// score_move(): the move scoring function
inline int score_move(int move);

// evaluate(): the board evaluation function
inline int evaluate();

// the search algorithm for finding the best move:
void search(int depth);
int negamax(int depth, int alpha, int beta);
int quiescence(int alpha, int beta);

#endif
