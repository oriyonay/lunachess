#include "engine.h"

static inline int evaluate(board* b) {
  // start with naive evaluation:
  int score = b->base_score;

  // add bishop pair bonus:
  if (b->bitboard[WB] & (b->bitboard[WB] - 1)) score += BISHOP_PAIR_BONUS;
  if (b->bitboard[BB] & (b->bitboard[BB] - 1)) score -= BISHOP_PAIR_BONUS;

  if (b->turn == WHITE) {
    //
    return score;
  }
  else {
    //
    return -score;
  }
}

// score_move(): the move scoring function
static inline int score_move(int move) {
  return MVV_LVA_SCORE[MOVE_PIECEMOVED(move)][MOVE_CAPTURED(move)];
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

  // score the moves:
  int move_scores[MAX_POSITION_MOVES];
  for (int i = 0; i < num_moves; i++) {
    move_scores[i] = score_move(moves[i]);
  }

  // recursively find the best move from here:
  int move, move_index;
  for (int i = 0; i < num_moves; i++) {
    // selection sort to find the best move:
    move_index = 0; // contains the index of the best move
    for (int j = 0; j < num_moves; j++) {
      if (move_scores[j] > move_scores[move_index]) {
        move_index = j;
      }
    }

    // once we find the move with the highest score, set its score to -1 so it
    // can't be selected again (essentially marking it as 'seen')
    move = moves[move_index];
    move_scores[move_index] = -1;

    b->make_move(move);
    candidate_score = -negamax_helper(depth - 1, alpha, beta);
    if (candidate_score > best_score) {
      best_score = candidate_score;
      best_move = move;
    }
    b->undo_move();

    alpha = std::max(alpha, best_score);
    if (alpha >= beta) break;
  }

  return best_move;
}

// negamax_helper(): the recursively-called helper function for negamax:
inline int search::negamax_helper(int depth, int alpha, int beta) {
  nodes_evaluated++;

  // base case:
  if (depth == 0) return quiescence(DEFAULT_QSEARCH_DEPTH, alpha, beta);

  // generate all possible moves from this position:
  int num_moves;
  int* moves = b->get_moves(num_moves);

  // if we can't make any moves, it's either checkmate or stalemate:
  // (we add the ply to -INF score to help the engine detect the CLOSEST
  // checkmate possible when it's defeating its opponent)
  if (num_moves == 0) return b->is_check() ? -INF + b->num_moves_played : 0;

  // score the moves:
  int move_scores[MAX_POSITION_MOVES];
  for (int i = 0; i < num_moves; i++) {
    move_scores[i] = score_move(moves[i]);
  }

  // recursively find the best move from here:
  int best_score = -INF;
  int move, move_index;
  for (int i = 0; i < num_moves; i++) {
    // selection sort to find the best move:
    move_index = 0; // contains the index of the best move
    for (int j = 0; j < num_moves; j++) {
      if (move_scores[j] > move_scores[move_index]) {
        move_index = j;
      }
    }

    // once we find the move with the highest score, set its score to -1 so it
    // can't be selected again (essentially marking it as 'seen')
    move = moves[move_index];
    move_scores[move_index] = -1;

    // make move & recursively call negamax helper function:
    b->make_move(move);
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
  nodes_evaluated++;

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

  // score the moves:
  int move_scores[MAX_POSITION_MOVES];
  for (int i = 0; i < num_moves; i++) {
    move_scores[i] = score_move(moves[i]);
  }

  // recursively qsearch the horizon:
  int best_score = -INF;
  int move, move_index;
  for (int i = 0; i < num_moves; i++) {
    // selection sort to find the best move:
    move_index = 0; // contains the index of the best move
    for (int j = 0; j < num_moves; j++) {
      if (move_scores[j] > move_scores[move_index]) {
        move_index = j;
      }
    }

    // once we find the move with the highest score, set its score to -1 so it
    // can't be selected again (essentially marking it as 'seen')
    move = moves[move_index];
    move_scores[move_index] = -1;

    // make move & recursively call negamax helper function:
    b->make_move(move);
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
