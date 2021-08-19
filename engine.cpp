// ENGINE.CPP/H: contains the 'intelligent' parts of the engine: search algorithms,
// move scoring/ordering, etc.
#include "engine.h"

// score_move(): the move scoring function
inline int score_move(int move) {
  // is this a PV move?
  if (score_pv && (move == pv_table[0][b.ply])) {
    score_pv = false;
    return 10000;
  }

  // is this move the best move for this position, as stored in our TT?
  if (move == (&TT[b.hash % NUM_TT_ENTRIES])->best_move) return 9000;

  // MVV/LVA scoring
  int score = MVV_LVA_SCORE[MOVE_PIECEMOVED(move)][MOVE_CAPTURED(move)] +
              (MOVE_IS_PROMOTION(move) ? 900 : 0);
  if (score) return score;

  // killer move heuristic for quiet moves:
  if (move == killer_moves[0][b.ply]) return 50;
  if (move == killer_moves[1][b.ply]) return 40;

  return history_moves[MOVE_PIECEMOVED(move)][MOVE_TO(move)];
}

// evaluate(): the board evaluation function
inline int evaluate() {
  // start with naive evaluation (b.base_score) and add bonus:
  int bonus = 0;

  // add bishop pair bonus:
  if (b.bitboard[WB] & (b.bitboard[WB] - 1)) bonus += BISHOP_PAIR_BONUS;
  if (b.bitboard[BB] & (b.bitboard[BB] - 1)) bonus -= BISHOP_PAIR_BONUS;

  // doubled pawn penalty:
  U64 white_pawn_file;
  U64 black_pawn_file;
  for (int file = 0; file < 8; file++) {
    white_pawn_file = (WP & FILES[file]);
    black_pawn_file = (BP & FILES[file]);
    if (white_pawn_file &&
       (white_pawn_file & (white_pawn_file - 1))) bonus -= DOUBLED_PAWN_PENALTY;
    if (black_pawn_file &&
       (black_pawn_file & (black_pawn_file - 1))) bonus += DOUBLED_PAWN_PENALTY;
  }

  // isolated and passed pawn penalty/bonus:
  U64 wp = b.bitboard[WP];
  U64 bp = b.bitboard[BP];
  int index;
  while (wp) {
    index = LSB(wp);
    if (!(b.bitboard[WP] & ISOLATED_MASKS[index])) bonus -= ISOLATED_PAWN_PENALTY;
    if (!(b.bitboard[BP] & WHITE_PASSED_PAWN_MASKS[index])) bonus += PASSED_PAWN_BONUS[RANK_NO(index)];
    POP_LSB(wp);
  }
  while (bp) {
    index = LSB(bp);
    if (!(b.bitboard[BP] & ISOLATED_MASKS[index])) bonus += ISOLATED_PAWN_PENALTY;
    if (!(b.bitboard[WP] & BLACK_PASSED_PAWN_MASKS[index])) bonus -= PASSED_PAWN_BONUS[9 - RANK_NO(index)];
    POP_LSB(bp);
  }

  // semi-open and fully-open rook files:
  U64 wr = b.bitboard[WR] | b.bitboard[WQ];
  U64 br = b.bitboard[BR] | b.bitboard[BQ];
  while (wr) {
    index = LSB(wr);
    if (!((b.bitboard[WP] | b.bitboard[BP]) & SQUARE_FILES[index])) bonus += FULLY_OPEN_FILE_BONUS;
    else if (!(b.bitboard[WP] & SQUARE_FILES[index])) bonus += SEMI_OPEN_FILE_BONUS;
    POP_LSB(wr);
  }
  while (br) {
    index = LSB(br);
    if (!((b.bitboard[WP] | b.bitboard[BP]) & SQUARE_FILES[index])) bonus -= FULLY_OPEN_FILE_BONUS;
    else if (!(b.bitboard[BP] & SQUARE_FILES[index])) bonus -= SEMI_OPEN_FILE_BONUS;
    POP_LSB(br);
  }

  // piece mobility evaluation:
  /* U64 not_pinned_white;
  U64 not_pinned_black;
  if (b.turn == WHITE) {
    not_pinned_white = ~(b.pinned_pieces());

    // get opponent's pinned pieces by flipping the turn and getting pinned pieces:
    b.turn = BLACK;
    not_pinned_black = ~(b.pinned_pieces());
    b.turn = WHITE;
  }
  else {
    not_pinned_black = ~(b.pinned_pieces());

    // get opponent's pinned pieces by flipping the turn and getting pinned pieces:
    b.turn = WHITE;
    not_pinned_white = ~(b.pinned_pieces());
    b.turn = BLACK;
  }

  U64 knights = b.bitboard[WN] & not_pinned_white;
  while (knights) {
    index = LSB(knights);
    bonus += (__builtin_popcountll(KNIGHT_MOVES[index] & ~b.W) - 4) * 4;
    POP_LSB(knights);
  }
  knights = b.bitboard[BN] & not_pinned_black;
  while (knights) {
    index = LSB(knights);
    bonus -= (__builtin_popcountll(KNIGHT_MOVES[index] & ~b.B) - 4) * 4;
    POP_LSB(knights);
  }
  U64 bishops = (b.bitboard[WB] | b.bitboard[WQ]) & not_pinned_white;
  while (bishops) {
    index = LSB(bishops);
    bonus += (__builtin_popcountll(diag_moves_magic(index, b.OCCUPIED_SQUARES)) - 6) * 5;
    POP_LSB(bishops);
  }
  bishops = (b.bitboard[BB] | b.bitboard[BQ]) & not_pinned_black;
  while (bishops) {
    index = LSB(bishops);
    bonus -= (__builtin_popcountll(diag_moves_magic(index, b.OCCUPIED_SQUARES)) - 6) * 5;
    POP_LSB(bishops);
  } */

  // king safety evaluation:
  bonus += __builtin_popcountll(KING_MOVES[LSB(b.bitboard[WK])] & b.bitboard[WP]) * KING_SHIELD_BONUS;
  bonus -= __builtin_popcountll(KING_MOVES[LSB(b.bitboard[BK])] & b.bitboard[BP]) * KING_SHIELD_BONUS;

  // calculate base score based on game phase (tapered evaluation):
  int base_score;
  if (b.game_phase_score > OPENING_PHASE_SCORE) base_score = b.base_score_opening;
  else if (b.game_phase_score < ENDGAME_PHASE_SCORE) base_score = b.base_score_endgame;
  else {
    base_score = (
                    b.base_score_opening * b.game_phase_score +
                    b.base_score_endgame * (OPENING_PHASE_SCORE - b.game_phase_score)
                 ) / OPENING_PHASE_SCORE;
  }

  // return the evaluation relative to the side whose turn it is:
  if (b.turn == WHITE) return base_score + bonus;
  else return -base_score - bonus;
}

// search(): the main search algorithm (with iterative deepening)
void search(int depth) {
  // zero counter of nodes searched:
  nodes_evaluated = 0;

  // reset score PV flag:
  score_pv = false;

  // reset time control flag:
  stop_search = false;

  // find best move within a given position
  int alpha = -INF;
  int beta = INF;
  int aspiration_delta = ASPIRATION_WINDOW_VALUE;
  for (int cur_depth = 1; cur_depth <= depth; cur_depth++) {
    // enable the follow_pv flag:
    follow_pv = true;

    // search the position at the given depth:
    int score = negamax(cur_depth, alpha, beta, 0);

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

    // if we still haven't looked 6 moves deep (not enough info to use windows), or
    // if we fell outside our aspiration window, reset to -INF and INF:
    if (cur_depth < 6 || ((score <= alpha) || (score >= beta))) {
      alpha = -INF;
      beta = INF;
      continue;
    }

    // otherwise, set alpha & beta using aspiration window value:
    alpha = score - aspiration_delta;
    beta = score + aspiration_delta;

    // gradually widen the search window in the future, if we failed high:
    aspiration_delta += (aspiration_delta / 2);

    // now the principal variation is in pv_table[0][:pv_length[0]],
    // and the best move is in pv_table[0][0]
  }

  // print the best move found:
  printf("bestmove ");
  print_move(pv_table[0][0]);
  printf("\n");
}

// negamax(): the main tree-search function
int negamax(int depth, int alpha, int beta, int forward_ply) {
  // update the UNSAFE bitboard, and a local is_check variable,
  // since it's not updating elsewhere for some reason:
  b.update_move_info_bitboards();
  bool is_check = b.is_check();

  // base case:
  if (depth <= 0 && !is_check) return quiescence(alpha, beta, forward_ply);

  // ensure non-negative depth:
  depth = std::max(depth, 0);

  // if this is a draw, return 0:
  // NOTE: we don't yet count material draws
  if (b.is_repetition() || b.fifty_move_counter >= 100) return 0;

  // every 2048 nodes, communicate with the GUI / check time:
  nodes_evaluated++;
  if (nodes_evaluated % 2048 == 0) communicate();
  if (stop_search) return evaluate();

  // avoid stack overflow:
  if (forward_ply >= MAX_SEARCH_PLY) return evaluate();

  pv_length[forward_ply] = forward_ply;
  bool pv = (beta - alpha) > 1;
  bool root = (forward_ply == 0);
  int score = -INF;
  int best_score = -INF;
  int best_move = NULL;
  char tt_flag = TT_ALPHA;

  // look up the position in the TT:
  score = probe_tt(depth, alpha, beta);
  if (b.ply > 0 && !pv && (score != TT_NO_MATCH)) return score;
  score = -INF;

  int eval = evaluate();
  static_evals[forward_ply] = eval;
  bool improving = (!is_check && forward_ply >= 2 && eval > static_evals[forward_ply-2]);

  // for null-move pruning, we need to figure out whether our side has any major pieces:
  U64 majors = (b.turn == WHITE) ?
                  b.bitboard[WN] | b.bitboard[WB] | b.bitboard[WR] | b.bitboard[WQ] :
                  b.bitboard[BN] | b.bitboard[BB] | b.bitboard[BR] | b.bitboard[BQ];

  // null-move pruning:
  if (!pv &&
      !is_check &&
      depth >= NULL_MOVE_PRUNING_DEPTH &&
      b.ply > 0 &&
      b.move_history[b.ply-1] != NULL &&
      majors
  ) {
    // give current side an extra turn:
    b.make_nullmove();

    // search the position with a reduced depth:
    int null_move_score = -negamax(depth - 4, -beta, -beta + 1, forward_ply + 1);

    // flip the turn again:
    b.undo_nullmove();

    // if we have to stop, stop the search:
    if (stop_search) return evaluate();

    // fail-hard beta cutoff:
    if (null_move_score >= beta) return beta;
  }

  // generate all possible moves from this position:
  int moves[MAX_POSITION_MOVES];
  int num_moves = b.get_moves(moves);

  // if we're following the principal variation, enable PV scoring:
  if (follow_pv) {
    follow_pv = false;
    for (int i = 0; i < num_moves; i++) {
      if (pv_table[0][forward_ply] == moves[i]) {
        score_pv = true;
        follow_pv = true;
      }
    }
  }

  // score the moves:
  int move_scores[MAX_POSITION_MOVES];
  for (int i = 0; i < num_moves; i++) {
    move_scores[i] = score_move(moves[i]);
  }

  // variables for move selection (selection sort, in main recursive loops):
  int move, move_index;

  // recursively find the best move from here:
  int non_pruned_moves = 0;
  bool tactical;
  bool is_killer;
  bool recapture;
  int extension;
  int new_depth;
  int R;
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

    // figure out some things about this move:
    tactical = (MOVE_CAPTURED(move) != NONE) || (MOVE_IS_PROMOTION(move));
    is_killer = (move == killer_moves[0][forward_ply]) ||
                (move == killer_moves[1][forward_ply]);
    recapture = (b.ply) &&
                (MOVE_CAPTURED(b.move_history[b.ply-1]) != NONE) &&
                (MOVE_CAPTURED(move) != NONE) &&
                (MOVE_TO(b.move_history[b.ply-1]) == MOVE_TO(move));

    // FUTURE: PRUNING GOES HERE

    non_pruned_moves++;

    // extensions:
    extension = 0;
    if (MOVE_IS_CASTLE(move) || (b.is_check() && depth < 8) || (pv && recapture))
      if (!root) extension = 1;
    new_depth = depth + extension;

    b.make_move(move);

    // reductions:
    R = 1;
    if (depth > 2 && non_pruned_moves > 1) {
      if (!tactical) {
        if (!b.is_check()) R += 2;
        if (!pv) R++;
        if (!improving) R++;
        if (is_killer) R -= 2;
      }
      else R -= pv ? 2 : 1;
      R = std::min(depth - 1, std::max(R, 1));
    }

    // PVS:
    if (pv && non_pruned_moves == 1) {
      score = -negamax(depth - 1, -beta, -alpha, forward_ply + 1);
    }
    else {
      score = -negamax(new_depth - R, -alpha - 1, -alpha, forward_ply + 1);
      if (R != 1 && score > alpha) {
        score = -negamax(new_depth - 1, -alpha - 1, -alpha, forward_ply + 1);
      }
      if ((score > alpha) && (score < beta)) {
        score = -negamax(new_depth - 1, -beta, -alpha, forward_ply + 1);
      }
    }

    // search first node at full-depth:
    /* if (i == 0) {
      score = -negamax(depth - 1, -beta, -alpha, forward_ply + 1);
    }
    // we check for potential LMR in all other nodes:
    else {
      // can we do LMR?
      if (i >= LMR_FULL_DEPTH_MOVES &&
          depth >= LMR_REDUCTION_LIMIT &&
          !is_check &&
          MOVE_CAPTURED(move) == NONE &&
          !MOVE_IS_PROMOTION(move)
      ) {
        score = -negamax(depth - R, -alpha - 1, -alpha, forward_ply + 1);
      }
      // if we can't, we use this trick to make sure we enter PVS and perform
      // the full search if necessary:
      else {
        score = alpha + 1;
      }

      // now we perform PVS:
      if (score > alpha) {
        score = -negamax(depth - 1, -alpha - 1, -alpha, forward_ply + 1);
        if ((score > alpha) && (score < beta)) {
          score = -negamax(depth - 1, -beta, -alpha, forward_ply + 1);
        }
      }
    } */
    b.undo_move();

    // if we have to stop, stop the search:
    if (stop_search) return 0;

    // if we found a better move (PV node):
    if (score > alpha) {
      // fail-hard beta cutoff (node fails high)
      if (score >= beta) {
        // store beta in the transposition table for this position:
        update_tt(depth, beta, move, TT_BETA);

        // add this move to the killer move list, only if it's a quiet move
        if (MOVE_CAPTURED(move) == NONE && (move != killer_moves[0][b.ply])) {
          killer_moves[1][b.ply] = killer_moves[0][b.ply];
          killer_moves[0][b.ply] = move;
        }

        return beta;
      }
      // switch the TT flag to indicate storing the exact value of this position,
      // since it's a PV node:
      tt_flag = TT_EXACT;

      // add this move to the history move list, only if it's a quiet move:
      if (MOVE_CAPTURED(move) == NONE) {
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

  if (num_moves == 0) return is_check ? -INF + forward_ply : 0;

  // node fails low. store the value in the transposition table first, then exit:
  best_move = pv_table[forward_ply][forward_ply];
  update_tt(depth, alpha, best_move, tt_flag);
  return alpha;
}

// quiescence(): the quiescence search algorithm
int quiescence(int alpha, int beta, int forward_ply) {
  // if this is a draw, return 0:
  // NOTE: we don't yet count material draws
  if (b.is_repetition() || b.fifty_move_counter >= 100) return 0;

  // every 2048 nodes, communicate with the GUI / check time:
  if (nodes_evaluated % 2048 == 0) communicate();

  // if we have to stop, stop the search:
  if (stop_search) return 0;

  // avoid stack overflow:
  if (forward_ply >= MAX_SEARCH_PLY) return evaluate();

  nodes_evaluated++;

  // update the UNSAFE bitboard, since it's not updating elsewhere for some reason:
  b.update_move_info_bitboards();

  // call negamax if we're in check, to make sure we don't get ourselves in a
  // mating net
  if (b.is_check()) return negamax(0, alpha, beta, forward_ply + 1);

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
    move_scores[i] = score_move(moves[i]);
  }

  // recursively qsearch the horizon:
  int move, move_index;
  int best_case;
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

    // make move & recursively call qsearch:
    b.make_move(move);
    int score = -quiescence(-beta, -alpha, forward_ply + 1);
    b.undo_move();

    // if we have to stop, stop the search:
    if (stop_search) return 0;

    // if we found a better move (PV node):
    if (score > alpha) {
      // fail-hard beta cutoff (node fails high)
      if (score >= beta) return beta;

      alpha = score;
    }
  }

  // node fails low:
  return alpha;
}
