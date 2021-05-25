/* ENGINE.H: contains the 'intelligent' part of the program, such as the board
 * evaluation function, the move ordering function, and the minimax search
 * algorithm for finding the best move.
*/

#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"

// a struct to store a move (int) and its respective evaluation (int):
struct move {
  int move_code;
  int score;

  move() {}
  move(int move_code, int score) : move_code(move_code), score(score) {}
};

// evaluate(): the board evaluation function
int evaluate(board* b);

// the move ordering function
// TODO

// the search algorithm for finding the best move:
move alphabeta(board* b, int depth);
int alphabeta_helper(board* b, int depth, int alpha, int beta, bool maximizing);

#endif
