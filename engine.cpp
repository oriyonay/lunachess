#include "engine.h"

static inline int evaluate(board* b) {
  // naive evaluation for now:
  return (b->turn == WHITE) ? b->base_score : -b->base_score;
}

search::search(board* b) : best_move(NULL), best_score(-INF), b(b) {}

int search::negamax(int depth) {
  nodes_evaluated = 0;
  // initialize important variables:
  int alpha = -INF;
  int beta = INF;
  int candidate_score;

  // generate all possible moves from this position:
  int num_moves;
  int* moves = b->get_moves(num_moves);

  // recursively find the best move from here:
  for (int i = 0; i < num_moves; i++) {
    b->make_move(moves[i]);
    candidate_score = -negamax_helper(depth - 1, alpha, beta);
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

// negamax_helper(): the recursively-called helper function for negamax:
inline int search::negamax_helper(int depth, int alpha, int beta) {
  // base case:
  if (depth == 0) { nodes_evaluated++; return quiescence(DEFAULT_QSEARCH_DEPTH, alpha, beta); }

  // generate all possible moves from this position:
  int num_moves;
  int* moves = b->get_moves(num_moves);

  // if we can't make any moves, it's either checkmate or stalemate:
  // (we add the ply to -INF score to help the engine detect the CLOSEST
  // checkmate possible when it's defeating its opponent)
  if (num_moves == 0) return b->is_check() ? -INF + b->num_moves_played : 0;

  // recursively find the best move from here:
  int best_score = -INF;
  for (int i = 0; i < num_moves; i++) {
    // make move & recursively call negamax helper function:
    b->make_move(moves[i]);
    int score = -negamax_helper(depth - 1, -beta, -alpha);
    b->undo_move();

    // fail-hard beta cutoff (node fails high)
    if (score >= beta) return beta;

    // if we found a better move (PV node):
    if (score > alpha) alpha = score;
  }

  // node fails low:
  return alpha;
}

// quiescence(): the quiescence search algorithm
inline int search::quiescence(int depth, int alpha, int beta) {
  // static evaluation:
  int eval = evaluate(b);

  // alpha/beta escape conditions:
  if (eval >= beta) return beta;
  if (eval > alpha) alpha = eval;

  // generate all possible moves from this position:
  int num_moves;
  int* moves = b->get_nonquiet_moves(num_moves);

  // check for checkmate/stalemate:
  if (num_moves == 0) return b->is_check() ? -INF + b->num_moves_played : 0;

  // recursively qsearch the horizon:
  int best_score = -INF;
  for (int i = 0; i < num_moves; i++) {
    // make sure to only search captures:
    if (MOVE_CAPTURED(moves[i]) == NONE) continue;

    // make move & recursively call negamax helper function:
    b->make_move(moves[i]);
    int score = -quiescence(depth - 1, -beta, -alpha);
    b->undo_move();

    // fail-hard beta cutoff (node fails high)
    if (score >= beta) return beta;

    // if we found a better move (PV node):
    if (score > alpha) alpha = score;
  }

  // node fails low:
  return alpha;
}
