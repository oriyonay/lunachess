/* ENGINE.H: contains the 'intelligent' part of the program, such as the board
 * evaluation function, the move ordering function, and the minimax search
 * algorithm for finding the best move.
*/

#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"

// evaluate(): the board evaluation function
int evaluate(board* b);

// the move ordering function
// TODO

// the search algorithm for finding the best move:
int negamax(board* b, int depth);
int negamax_helper(board* b, int depth, int alpha, int beta, bool maximizing);

#endif
