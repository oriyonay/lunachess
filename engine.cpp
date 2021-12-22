// ENGINE.CPP/H: contains the 'intelligent' parts of the engine: search algorithms,
// move scoring/ordering, etc.
#include "engine.h"

// score_move(): the move scoring function
int score_move(int move, int forward_ply) {
  // is this a PV move?
  if (score_pv && (move == pv_table[0][forward_ply])) {
    score_pv = false;
    return 10000;
  }

  // is this move the best move for this position, as stored in our TT?
  if (move == (&TT.TT[b.hash & TT.mask])->best_move) return 9000;

  // is this a recapture?
  /* if ((b.ply) &&
      (MOVE_CAPTURED(b.move_history[b.ply-1]) != NONE) &&
      (MOVE_CAPTURED(move) != NONE) &&
      (MOVE_TO(b.move_history[b.ply-1]) == MOVE_TO(move))
  ) return 8000; */

  // in the endgame, is this a pawn push?
  // TODO

  // MVV/LVA scoring
  int score = MVV_LVA_SCORE[MOVE_PIECEMOVED(move)][MOVE_CAPTURED(move)] +
              (MOVE_IS_PROMOTION(move) ? 900 : 0);
  if (score) return score;
  /* if (MOVE_CAPTURED(move) != NONE) return see(move);
  if (MOVE_IS_PROMOTION(move)) return 1000; */

  // killer move heuristic for quiet moves:
  if (move == killer_moves[0][forward_ply] ||
      move == killer_moves[1][forward_ply]) return 50;

  return history_moves[MOVE_PIECEMOVED(move)][MOVE_TO(move)];
}

// search(): the main search algorithm (with iterative deepening)
void search(int depth) {
  // after this search, increment the transposition table's age:
  TT.age++;

  // zero counter of nodes searched, score-PV flag, and time control flag:
  nodes_evaluated = 0;
  score_pv = false;
  stop_search = false;

  // zero pv, killer move, history, and static eval tables:
  memset(pv_table, 0, sizeof(int) * MAX_SEARCH_PLY * MAX_SEARCH_PLY);
  memset(pv_length, 0, sizeof(int) * MAX_SEARCH_PLY);
  memset(killer_moves, 0, sizeof(int) * 2 * MAX_SEARCH_PLY);
  memset(history_moves, 0, sizeof(int) * 12 * 64);
  memset(static_evals, 0, sizeof(int) * MAX_SEARCH_PLY);

  // find best move in this position
  int alpha = -INF;
  int beta = INF;
  // int aspiration_delta = ASPIRATION_WINDOW_VALUE;
  for (int cur_depth = 1; cur_depth <= depth; cur_depth++) {
    // enable the follow_pv flag:
    follow_pv = true;

    // search the position at the given depth:
    int score = negamax(cur_depth, alpha, beta, 0, true);

    // if we have to stop, stop the search:
    if (stop_search) break;

    // print info to console:
    printf("info score cp %d depth %d nodes %d time %d pv ",
      score, cur_depth, nodes_evaluated, get_time() - start_time
    );
    for (int i = 0; i < pv_length[0]; i++) {
      print_move(pv_table[0][i]);
      printf(" ");
    }
    printf("\n");

    // if searching up to this ply took > 1/2 of our allocated time, terminate prematurely:
    // if ((time_limit != -1) && ((get_time() - start_time) > (time_limit / 2))) break;

    // if we still haven't looked 6 moves deep (not enough info to use windows), or
    // if we fell outside our aspiration window, reset to -INF and INF:
    /* if (cur_depth < 6 || ((score <= alpha) || (score >= beta))) {
      alpha = -INF;
      beta = INF;
      continue;
    } */

    // otherwise, set alpha & beta using aspiration window value:
    // alpha = score - aspiration_delta;
    // beta = score + aspiration_delta;

    // gradually widen the search window in the future, if we failed high:
    // aspiration_delta += (aspiration_delta / 2);

    // now the principal variation is in pv_table[0][:pv_length[0]],
    // and the best move is in pv_table[0][0]
  }

  // print the best move found:
  printf("bestmove ");
  print_move(pv_table[0][0]);
  printf("\n");
}

// negamax(): the main tree-search function
int negamax(int depth, int alpha, int beta, int forward_ply, bool forward_prune) {
  // every 2048 nodes, communicate with the GUI / check time:
  if ((nodes_evaluated & 2047) == 0) communicate();
  if (stop_search) return 0;
  nodes_evaluated++;

  // if this is a draw, return 0:
  if (b.is_repetition() || b.is_material_draw() || b.fifty_move_counter >= 100) return 0;

  pv_length[forward_ply] = forward_ply;
  bool pv = (beta - alpha != 1);
  bool root = (forward_ply == 0);
  int score = -INF;
  int best_score = -INF;
  int best_move = NULL;
  char tt_flag = TT_ALPHA;

  // look up the position in the TT:
  score = TT.probe(depth, alpha, beta);
  if (b.ply > 0 && !pv && (score != TT_NO_MATCH)) return score;
  score = -INF;

  // TODO: EGTB lookup goes here

  // update the UNSAFE bitboard and make a local is_check variable:
  b.update_move_info_bitboards();
  bool is_check = b.is_check();

  // base case:
  if (depth <= 0 && !is_check) return quiescence(alpha, beta, forward_ply);

  // store static eval in static eval table:
  int eval = evaluate();
  static_evals[forward_ply] = eval;
  bool improving = (!is_check && forward_ply >= 2 && eval > static_evals[forward_ply-2]);

  // check extension:
  if (is_check) depth = std::max(depth+1, 1);

  // ensure non-negative depth:
  depth = std::max(depth, 0);

  // avoid stack overflow:
  if (forward_ply >= MAX_SEARCH_PLY) return is_check ? 0 : eval;

  /* ---------- FORWARD PRUNING ---------- */

  // for null-move pruning, we need to figure out whether our side has any major pieces:
  U64 majors = (b.turn == WHITE) ?
                  b.bitboard[WN] | b.bitboard[WB] | b.bitboard[WR] | b.bitboard[WQ] :
                  b.bitboard[BN] | b.bitboard[BB] | b.bitboard[BR] | b.bitboard[BQ];

  bool failed_null = false;

  if (forward_prune &&
      !pv &&
      !is_check &&
      b.ply &&
      majors &&
      beta > -MATE_IN_MAX
  ) {
    // reverse futility pruning:
    int pessimistic_eval = eval - (75 * depth) - (100 * improving);
    if (depth < 3 &&
        pessimistic_eval >= beta
    ) return pessimistic_eval;

    // null-move pruning:
    if (depth >= NULL_MOVE_PRUNING_DEPTH &&
        b.move_history[b.ply-1] != NULL
    ) {
      // give current side an extra turn:
      b.make_nullmove();

      // search the position with a reduced depth:
      int null_move_score = -negamax(depth - 4, -beta, -beta + 1, forward_ply + 1, false);

      // flip the turn again:
      b.undo_nullmove();

      // if we have to stop, stop the search:
      if (stop_search) return 0;

      // fail-hard beta cutoff:
      if (null_move_score >= beta) return beta;

      // otherwise, we failed null-move pruning:
      failed_null = true;
    }

    // razoring:
    if (depth == 1 &&
        eval + 200 < beta
     )  return quiescence(alpha, beta, forward_ply);
  }

  /* ---------- END OF FORWARD PRUNING ---------- */

  // generate all possible moves from this position:
  int moves[MAX_POSITION_MOVES];
  int num_moves = b.get_moves(moves);

  // set the best move to be the first move on the list, just so we don't return a NULL-move
  // in case no move raises alpha:
  if (num_moves) best_move = moves[0];

  // if we're following the principal variation, enable PV scoring:
  if (follow_pv) {
    follow_pv = false;
    for (int i = 0; i < num_moves; i++) {
      if (pv_table[0][forward_ply] == moves[i]) {
        score_pv = true;
        follow_pv = true;
        break;
      }
    }
  }

  // score the moves:
  int move_scores[MAX_POSITION_MOVES];
  for (int i = 0; i < num_moves; i++) {
    move_scores[i] = score_move(moves[i], forward_ply);
  }

  // variables for move selection (selection sort, in main recursive loops):
  int move, move_index;

  // recursively find the best move from here:
  int non_pruned_moves = 0;
  int num_quiets = 0;
  bool tactical;
  bool is_killer;
  bool recapture;
  bool skip_quiets = false;
  int extension;
  int R;
  for (int i = 0; i < num_moves; i++) {
    // selection sort to find the best move:
    move_index = 0; // contains the index of the best move
    for (int j = 0; j < num_moves; j++) {
      if (move_scores[j] > move_scores[move_index]) {
        move_index = j;
      }
    }

    // once we find the move with the highest score, set its score to -INF so it
    // can't be selected again (essentially marking it as 'seen')
    move = moves[move_index];
    move_scores[move_index] = -INF;

    // figure out some things about this move:
    tactical = (MOVE_CAPTURED(move) != NONE) || (MOVE_IS_PROMOTION(move));
    is_killer = (move == killer_moves[0][forward_ply]) ||
                (move == killer_moves[1][forward_ply]);
    recapture = (b.ply) &&
                (MOVE_CAPTURED(b.move_history[b.ply-1]) != NONE) &&
                (MOVE_CAPTURED(move) != NONE);

    // ----- move skipping: ----- //

    /* if (!pv &&
        !is_check &&
        best_score > -MATE_IN_MAX &&
        non_pruned_moves > 1
    ) {

      // late move pruning:
      if (num_quiets > LMP_ARRAY[depth][improving]) skip_quiets = true;

      // extended futility pruning:
      if (depth < 3 &&
          !tactical &&
          eval + (75 * depth) - (100 * improving) <= alpha
      ) continue;
    }

    // skip quiet moves if this move is quiet and skip_quiets flag is on:
    if (skip_quiets && !tactical && (non_pruned_moves > 1)) continue; */

    // ----- end of move skipping ----- //

    non_pruned_moves++;
    // if (!tactical) num_quiets++;
    b.make_move(move);

    // extensions:
    /* extension = 0;

    // general extensions:
    if (MOVE_IS_CASTLE(move) || is_check || recapture)
      if (!root) extension++;

    // last capture extension: extend any non-pawn capture in the endgame:
    if (tactical &&
        b.game_phase_score < ENDGAME_PHASE_SCORE &&
        MOVE_CAPTURED(move) - b.turn != PAWN
    ) extension++;

    // passed pawn push extension:
    if (depth < 5 &&
        b.game_phase_score < ENDGAME_PHASE_SCORE &&
        PIECE_TYPE(MOVE_PIECEMOVED(move)) == PAWN
    ) extension++;

    new_depth = depth + extension; */

    // late-move reductions:
    R = 1;
    /* if (depth > 2 && non_pruned_moves > 2) {
      if (!tactical) {
        // if (!b.is_check() && non_pruned_moves >= LMR_FULL_DEPTH_MOVES) R += 2;
        // if (!pv) R++;
        // if (num_quiets > 3 && failed_null) R++;
        // if (!improving) R++;
        // if (MOVE_IS_PROMOTION(move) && PIECE_TYPE(MOVE_PROMOTION_PIECE(move)) == QUEEN) R--;
        // if (is_killer) R--;
      }
      else {
        // R -= pv ? 2 : 1;
        // if (see(move) < 0) R++; // NOTE: WE NEED TO PRECALCULATE SEE BEFORE MAKING THE MOVE!!
      }
      R = std::min(depth - 1, std::max(R, 1));
    } */

    // PVS:
    if (non_pruned_moves == 1) {
      score = -negamax(depth - 1, -beta, -alpha, forward_ply + 1, true);
    }
    else {
      score = -negamax(depth - R, -alpha - 1, -alpha, forward_ply + 1, true);
      if ((R != 1) && (score > alpha)) {
        score = -negamax(depth - 1, -alpha - 1, -alpha, forward_ply + 1, true);
      }
      if ((score > alpha) && (score < beta)) {
        score = -negamax(depth - 1, -beta, -alpha, forward_ply + 1, true);
      }
    }

    b.undo_move();

    if (stop_search) return 0;

    if (score > best_score) {
      best_score = score;
      best_move = move;

      if (score > alpha) {
        // fail-hard beta cutoff (node fails high)
        if (score >= beta) {
          // store beta in the transposition table for this position:
          TT.put(depth, beta, move, TT_BETA, true);

          // add this move to the killer move list, only if it's a quiet move
          if (move != NULL && !tactical && (move != killer_moves[0][forward_ply])) {
            killer_moves[1][forward_ply] = killer_moves[0][forward_ply];
            killer_moves[0][forward_ply] = move;
          }

          return beta;
        }

        // switch the TT flag to indicate storing the exact value of this position,
        // since it's a PV node:
        tt_flag = TT_EXACT;

        // add this move to the history move list, only if it's a quiet move:
        if (MOVE_CAPTURED(move) == NONE && (move != NULL)) {
          history_moves[MOVE_PIECEMOVED(move)][MOVE_TO(move)] += depth;
        }

        // this is the PV node:
        alpha = score;

        // enter PV move into PV table:
        pv_table[forward_ply][forward_ply] = move;

        // copy move from deeper PV:
        for (int next = forward_ply + 1; next < pv_length[forward_ply + 1]; next++) {
          pv_table[forward_ply][next] = pv_table[forward_ply + 1][next];
        }

        // adjust the PV length table:
        pv_length[forward_ply] = pv_length[forward_ply + 1];
      }
    }
  }

  if (num_moves == 0) return is_check ? -INF + forward_ply : 0;

  // node fails low. store the value in the transposition table first, then exit:
  TT.put(depth, alpha, best_move, tt_flag, false);
  return alpha;
}

// quiescence(): the quiescence search algorithm
int quiescence(int alpha, int beta, int forward_ply) {
  // every 2048 nodes, communicate with the GUI / check time:
  if ((nodes_evaluated & 2047) == 0) communicate();
  nodes_evaluated++;

  // avoid stack overflow:
  if (forward_ply >= MAX_SEARCH_PLY) return evaluate();

  // call negamax if we're in check, to make sure we don't get ourselves in a
  // mating net
  b.update_move_info_bitboards();
  if (b.is_check()) return negamax(0, alpha, beta, forward_ply, false);

  // if this is a draw, return 0:
  if (b.is_repetition() || b.is_material_draw() || b.fifty_move_counter >= 100) return 0;

  // do we have this position stored in the TT? if so, use it:
  // int tt_score = TT.probe(0, alpha, beta);
  // if (b.ply > 0 && (tt_score != TT_NO_MATCH)) return tt_score;

  // static evaluation:
  int eval = evaluate();

  // alpha/beta escape conditions:
  if (eval >= beta) return beta;
  if (eval > alpha) alpha = eval;

  // generate all possible moves from this position:
  int moves[MAX_POSITION_MOVES];
  int num_moves = b.get_nonquiet_moves(moves);

  // score the moves:
  int move_scores[MAX_POSITION_MOVES];
  for (int i = 0; i < num_moves; i++) {
    move_scores[i] = score_move(moves[i], forward_ply);
  }

  // recursively qsearch the horizon:
  int move, move_index;
  int best_case;
  int score;
  for (int i = 0; i < num_moves; i++) {
    // selection sort to find the best move:
    move_index = 0; // contains the index of the best move
    for (int j = 0; j < num_moves; j++) {
      if (move_scores[j] > move_scores[move_index]) {
        move_index = j;
      }
    }

    // once we find the move with the highest score, set its score to -INF so it
    // can't be selected again (essentially marking it as 'seen')
    move = moves[move_index];
    move_scores[move_index] = -INF;

    // delta pruning: could this move improve alpha, in the best case?
    // best_case = std::max(see(move), DELTA_VALUE);
    // if (eval + best_case <= alpha) continue;

    // make move & recursively call qsearch:
    b.make_move(move);
    score = -quiescence(-beta, -alpha, forward_ply + 1);
    b.undo_move();

    // if we have to stop, stop the search:
    if (stop_search) return eval;

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
