#include "engine.h"

// init_search(): initialize search variables
inline void init_search() {
  // zero pv, killer move, and history tables:
  memset(pv_table, 0, sizeof(int) * MAX_GAME_MOVES * MAX_GAME_MOVES);
  memset(killer_moves, 0, sizeof(int) * 2 * MAX_GAME_MOVES);
  memset(history_moves, 0, sizeof(int) * 12 * 64);

  // zero counter of nodes searched:
  nodes_evaluated = 0;
}

// score_move(): the move scoring function
inline int score_move(int move) {
  // is this a PV move?
  if (move == pv_table[0][b.num_moves_played]) return 1000;

  // MVV/LVA scoring
  int score = MVV_LVA_SCORE[MOVE_PIECEMOVED(move)][MOVE_CAPTURED(move)];
  if (score) return score;

  // killer move heuristic for quiet moves:
  if (move == killer_moves[0][b.num_moves_played]) return 50;
  if (move == killer_moves[1][b.num_moves_played]) return 40;

  return history_moves[MOVE_PIECEMOVED(move)][MOVE_TO(move)];
}

// evaluate(): the board evaluation function
inline int evaluate() {
  // start with naive evaluation:
  int score = b.base_score;

  // add bishop pair bonus:
  if (b.bitboard[WB] & (b.bitboard[WB] - 1)) score += BISHOP_PAIR_BONUS;
  if (b.bitboard[BB] & (b.bitboard[BB] - 1)) score -= BISHOP_PAIR_BONUS;

  if (b.turn == WHITE) {
    //
    return score;
  }
  else {
    //
    return -score;
  }
}

// search(): the main search algorithm (with iterative deepening)
void search(int depth) {
  // clear search helper tables:
  init_search();

  // record starting time:
  int start_time = get_time();

  // find best move within a given position
  int alpha = -INF;
  int beta = INF;
  int ply = b.num_moves_played;
  for (int cur_depth = 1; cur_depth <= depth; cur_depth++) {
    // search the position at the given depth:
    int score = negamax(cur_depth, alpha, beta);

    // print info to console:
    printf("info score cp %d depth %d nodes %d time %d pv ",
      score, cur_depth, nodes_evaluated, get_time() - start_time
    );
    for (int i = ply; i < pv_length[ply]; i++) {
      print_move(pv_table[ply][i]);
      printf(" ");
    }
    printf("\n");

    // if we fell outside our aspiration window, reset to -INF and INF:
    if ((score <= alpha) || (score >= beta)) {
      alpha = -INF;
      beta = INF;
      continue;
    }

    // otherwise, set alpha & beta using aspiration window value:
    alpha = score - ASPIRATION_WINDOW_VALUE;
    beta = score + ASPIRATION_WINDOW_VALUE;

    // now the principal variation is in pv_table[0][:pv_length[0]],
    // and the best move is in pv_table[ply][0]
  }

  // print the best move found:
  printf("bestmove ");
  print_move(pv_table[ply][ply]);
  printf("\n");
}

// negamax_helper(): the recursively-called helper function for negamax
int negamax(int depth, int alpha, int beta) {
  nodes_evaluated++;

  // initialize PV length:
  pv_length[b.num_moves_played] = b.num_moves_played;

  // base case:
  if (depth == 0) return evaluate(); // quiescence(DEFAULT_QSEARCH_DEPTH, alpha, beta);

  // null-move pruning:
  /* if (!b.is_check() && depth >= 3) {
    // give current side an extra turn:
    b.turn = (b.turn == WHITE) ? BLACK : WHITE;
    b.move_history[b.num_moves_played++] = NULL;

    // search the position with a reduced depth:
    int null_move_score = -negamax(depth - 3, -beta, -beta + 1);

    // flip the turn again:
    b.turn = (b.turn == WHITE) ? BLACK : WHITE;
    b.num_moves_played--;

    // fail-hard beta cutoff:
    if (null_move_score >= beta) return beta;
  } */

  // generate all possible moves from this position:
  int num_moves;
  int* moves = b.get_moves(num_moves);

  // check extension:
  // note: since we just called board::get_moves(), the board::UNSAFE bitboard
  // is updated automatically
  if (b.is_check()) depth++;

  // if we can't make any moves, it's either checkmate or stalemate:
  // (we add the ply to -INF score to help the engine detect the CLOSEST
  // checkmate possible when it's defeating its opponent)
  if (num_moves == 0) {
    b.update_unsafe();
    return b.is_check() ? -INF + b.num_moves_played : 0;
  }

  // score the moves:
  int move_scores[MAX_POSITION_MOVES];
  for (int i = 0; i < num_moves; i++) {
    move_scores[i] = score_move(moves[i]);
  }

  // recursively find the best move from here:
  int best_score = -INF;
  int move, move_index;
  bool found_pv = false;
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
    // we also implement the meat of PVS in this section:
    b.make_move(move);
    int score;
    // search first node at full-depth:
    if (i == 0) {
      score = -negamax(depth - 1, -beta, -alpha);
    }
    // we check for potential LMR in all other nodes:
    else {
      // can we do LMR?
      if (i >= LMR_FULL_DEPTH_MOVES &&
          depth >= LMR_REDUCTION_LIMIT &&
          !b.is_check() &&
          MOVE_CAPTURED(move) == NONE &&
          !MOVE_IS_PROMOTION(move)
      ) {
        score = -negamax(depth - 2, -alpha - 1, -alpha);
      }
      // if we can't, we use this trick to make sure we enter PVS and perform
      // the full search if necessary:
      else {
        score = alpha + 1;
      }

      // now we perform PVS:
      if (score > alpha) {
        score = -negamax(depth - 1, -alpha - 1, -alpha);
        if ((score > alpha) && (score < beta)) {
          score = -negamax(depth - 1, -beta, -alpha);
        }
      }
      else {
        score = -negamax(depth - 1, -beta, -alpha);
      }
    }
    b.undo_move();

    // fail-hard beta cutoff (node fails high)
    if (score >= beta) {
      // add this move to the killer move list, only if it's a quiet move
      if (MOVE_CAPTURED(move) == NONE) {
        killer_moves[1][b.num_moves_played] = killer_moves[0][b.num_moves_played];
        killer_moves[0][b.num_moves_played] = move;
      }

      return beta;
    }

    // if we found a better move (PV node):
    if (score > alpha) {
      // add this move to the history move list, only if it's a quiet move:
      if (MOVE_CAPTURED(move) == NONE) {
        history_moves[MOVE_PIECEMOVED(move)][MOVE_TO(move)] += depth;
      }

      // this is the PV node:
      alpha = score;

      // since we found a PV node, enable the found_pv flag:
      found_pv = true;

      // enter PV move into PV table:
      int ply = b.num_moves_played;
      pv_table[ply][ply] = move;

      // copy move from deeper PV:
      for (int next = ply + 1; next < pv_length[ply + 1]; next++) {
        pv_table[ply][next] = pv_table[ply+1][next];
      }

      // adjust the PV length table:
      pv_length[ply] = pv_length[ply+1];
    }
  }

  // node fails low:
  return alpha;
}

// quiescence(): the quiescence search algorithm
int quiescence(int depth, int alpha, int beta) {
  nodes_evaluated++;

  // static evaluation:
  int eval = evaluate();

  // alpha/beta escape conditions:
  if (eval >= beta) return beta;
  if (eval > alpha) alpha = eval;

  // generate all possible moves from this position:
  int num_moves;
  int* moves = b.get_nonquiet_moves(num_moves);

  // check for checkmate/stalemate:
  if (num_moves == 0) {
    b.update_unsafe();
    return b.is_check() ? -INF + b.num_moves_played : 0;
  }

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
    b.make_move(move);
    int score = -quiescence(depth - 1, -beta, -alpha);
    b.undo_move();

    // if we found a better move (PV node):
    if (score > alpha) {
      alpha = score;

      // fail-hard beta cutoff (node fails high)
      if (score >= beta) return beta;
    }
  }

  // node fails low:
  return alpha;
}
