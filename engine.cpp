#include "engine.h"

int evaluate(board* b) {
  // naive evaluation for now:
  return b->material * ((b->turn == WHITE) ? 1 : -1);
}

int negamax(board* b, int depth) {
  // initialize important variables:
  int best_move = NULL;
  int best_score = -INF;
  int alpha = -INF;
  int beta = INF;
  int candidate_score;
  bool maximizing = (b->turn == WHITE);

  // recursively find the best move from here:
  int num_moves;
  int* moves = b->get_moves(num_moves);
  for (int i = 0; i < num_moves; i++) {
    b->make_move(moves[i]);
    candidate_score = -negamax_helper(b, depth - 1, alpha, beta, maximizing);
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

int negamax_helper(board* b, int depth, int alpha, int beta, bool maximizing) {
  // base case:
  if (depth == 0) return evaluate(b);

  // generate all possible moves from this position:
  int num_moves;
  int* moves = b->get_moves(num_moves);

  // if we can't make any moves, it's either checkmate or stalemate:
  if (num_moves == 0) return b->is_check() ? -INF : 0;

  // recursively find the best move from here:
  int best_score = -INF;
  for (int i = 0; i < num_moves; i++) {
    b->make_move(moves[i]);
    best_score = std::max(best_score, -negamax_helper(b, depth - 1, -beta, -alpha, !maximizing));
    b->undo_move();
    alpha = std::max(alpha, best_score);
    if (alpha >= beta) break;
  }

  return best_score;
}
