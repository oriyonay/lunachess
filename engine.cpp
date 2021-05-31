#include "engine.h"

// TODO: remove this
// TODO: remove nodes_evaluated
char* pm(int m) {
  int to = (m >> 26) & 0x3F;
  int from = (m >> 20) & 0x3F;

  char* move_str = new char[6];
  move_str[5] = '\0';
  move_str[4] = '\0'; // TODO: REMOVE THIS

  move_str[0] = ((from % 8) + 'a');
  move_str[1] = (8 - (from / 8) + '0');
  move_str[2] = ((to % 8) + 'a');
  move_str[3] = (8 - (to / 8) + '0');

  return move_str;
}

inline int evaluate(board* b) {
  // naive evaluation for now:
  return (b->turn == WHITE) ? b->material : -b->material;
}

/* int search::iterative_deepening(int max_depth) {
  for (int depth = 1; depth < max_depth; depth++) {
    negamax(depth);
  }
  return best_move;
} */

search::search(board* b) : abort(false), best_move(NULL), best_score(-INF), b(b) {}

int search::negamax(int depth) {
  nodes_evaluated = 0;
  // initialize important variables:
  int alpha = -INF;
  int beta = INF;
  int candidate_score;
  bool maximizing = (b->turn == WHITE);

  // generate all possible moves from this position:
  int num_moves;
  int* moves = b->get_moves(num_moves);

  // recursively find the best move from here:
  for (int i = 0; i < num_moves; i++) {
    b->make_move(moves[i]);
    candidate_score = -negamax_helper(depth - 1, alpha, beta, maximizing);
    if (candidate_score >= best_score) {
      best_score = candidate_score;
      best_move = moves[i];
    }
    b->undo_move();

    alpha = std::max(alpha, best_score);
    if (alpha >= beta) break;
  }

  return best_move;
}

int search::negamax_helper(int depth, int alpha, int beta, bool maximizing) {
  // if we got the abort signal, abort:
  if (abort) return 0;

  // base case:
  if (depth == 0) { nodes_evaluated++; return evaluate(b); }

  // generate all possible moves from this position:
  int num_moves;
  int* moves = b->get_moves(num_moves);

  // if we can't make any moves, it's either checkmate or stalemate:
  if (num_moves == 0) return b->is_check() ? -INF : 0;

  // recursively find the best move from here:
  int best_score = -INF;
  for (int i = 0; i < num_moves; i++) {
    b->make_move(moves[i]);
    best_score = std::max(best_score, -negamax_helper(depth - 1, -beta, -alpha, !maximizing));
    b->undo_move();
    alpha = std::max(alpha, best_score);
    if (alpha >= beta) break;
  }

  return best_score;
}

void search::stop() {
  abort = true;
}
