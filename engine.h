/* ENGINE.H: contains the 'intelligent' part of the program, such as the board
 * evaluation function, the move ordering function, and the minimax search
 * algorithm for finding the best move.
*/

#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"

// evaluate(): the board evaluation function
inline int evaluate();

// search: a class for searching a board
class search {
public:
  int nodes_evaluated;
  search(board* b);

  // the search algorithm for finding the best move:
  // int iterative_deepening(int max_depth);
  int negamax(int depth);
  int negamax_helper(int depth, int alpha, int beta);

  // to stop the search at any time:
  void stop();

private:
  bool abort;
  int best_move;
  int best_score;
  board* b;
};

#endif
