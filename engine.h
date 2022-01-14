/* ENGINE.H: contains the 'intelligent' part of the program, such as the move
 * scoring function, and the negamax search algorithm for finding the best move.
*/

#ifndef ENGINE_H
#define ENGINE_H

#include "syzygy/tbprobe.h"

#include "board.h"
#include "eval.h"
#include "globals.h"
#include "utils.h"

void search(int depth);
int negamax(int depth, int alpha, int beta, int forward_ply, bool forward_prune);
int quiescence(int alpha, int beta, int forward_ply);
int score_move(int move, int forward_ply);

#endif
