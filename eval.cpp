#include "eval.h"

// evaluate(): the board evaluation function
int evaluate() {
  // start with naive evaluation (b.base_score) and add bonus:
  int bonus = 0;

  // bishop pair bonus:
  if (SEVERAL(b.bitboard[WB])) bonus += BISHOP_PAIR_BONUS;
  if (SEVERAL(b.bitboard[BB])) bonus -= BISHOP_PAIR_BONUS;

  // doubled pawn penalty:
  int white_pawns_on_file;
  int black_pawns_on_file;
  for (int file = 0; file < 8; file++) {
    white_pawns_on_file = __builtin_popcountll(b.bitboard[WP] & FILES[file]);
    black_pawns_on_file = __builtin_popcountll(b.bitboard[BP] & FILES[file]);
    if (white_pawns_on_file > 1) bonus -= DOUBLED_PAWN_PENALTY; // * (white_pawns_on_file - 1);
    if (black_pawns_on_file > 1) bonus += DOUBLED_PAWN_PENALTY; // * (black_pawns_on_file - 1);
  }

  // isolated and passed pawn penalty/bonus:
  /* U64 wp = b.bitboard[WP];
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
  } */

  // semi-open and fully-open rook files:
  int index;
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
    bonus += (__builtin_popcountll(KNIGHT_MOVES[index] & ~b.W) - 4) * 6;
    POP_LSB(knights);
  }
  knights = b.bitboard[BN] & not_pinned_black;
  while (knights) {
    index = LSB(knights);
    bonus -= (__builtin_popcountll(KNIGHT_MOVES[index] & ~b.B) - 4) * 6;
    POP_LSB(knights);
  } */
  /* U64 bishops = (b.bitboard[WB] | b.bitboard[WQ]) & not_pinned_white;
  while (bishops) {
    index = LSB(bishops);
    bonus += (__builtin_popcountll(diag_moves_magic(index, b.OCCUPIED_SQUARES)) - 6) * 8;
    POP_LSB(bishops);
  }
  bishops = (b.bitboard[BB] | b.bitboard[BQ]) & not_pinned_black;
  while (bishops) {
    index = LSB(bishops);
    bonus -= (__builtin_popcountll(diag_moves_magic(index, b.OCCUPIED_SQUARES)) - 6) * 8;
    POP_LSB(bishops);
  } */

  // king safety evaluation:
  // bonus += __builtin_popcountll(KING_MOVES[LSB(b.bitboard[WK])] & b.bitboard[WP]) * KING_SHIELD_BONUS;
  // bonus -= __builtin_popcountll(KING_MOVES[LSB(b.bitboard[BK])] & b.bitboard[BP]) * KING_SHIELD_BONUS;

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
  return (base_score + bonus) * (b.turn == WHITE ? 1 : -1);
}

// see(): static exchange evaluation - is this static exchange going to be good for us?
/* int see(int move) {
  // unpack move info:
  int to = MOVE_TO(move);
  int from = MOVE_FROM(move);
  int next_victim = (MOVE_IS_PROMOTION(move)) ?
      MOVE_PROMOTION_PIECE(move) :
      b.piece_board[from];

  // declare the move balance score:
  int balance = estimated_move_value(move);

  // if the balance is losing as-is, we terminate early:
  if (balance < 0) return balance;

  // worst case is losing the capturing piece.
  // if the balance is positive then we can stop the exchange here and be up material
  // from the exchange, so we can stop early as well:
  if (balance - SEE_PIECE_VALUES[next_victim] > 0) return balance;

  // get sliders to help update hidden attackers:
  U64 bishops = b.bitboard[WB] | b.bitboard[BB] | b.bitboard[WQ] | b.bitboard[BQ];
  U64 rooks   = b.bitboard[WR] | b.bitboard[BR] | b.bitboard[WQ] | b.bitboard[BQ];

  // get OCCUPIED board and make the move on it:
  U64 occupied = (b.OCCUPIED_SQUARES ^ (1L << from)) | (1L << to);
  if (MOVE_IS_EP(move)) {
    char ep_square = (b.turn == WHITE) ? to + 8 : to - 8;
    occupied ^= (1L << ep_square);
  }

  // get all attackers for this square:
  U64 attackers = b.get_attackers(occupied, to) & occupied;

  char turn = (b.turn == WHITE) ? BLACK : WHITE;

  // now we do SEE:
  U64 my_attackers;
  int previous_attacker = b.piece_board[from];
  do {
    // if we don't have any more attackers, we lose:
    my_attackers = attackers & (turn == WHITE ? b.W : b.B);
    if (!my_attackers) break;

    // find weakest piece to attack with:
    for (next_victim = PAWN; next_victim <= QUEEN; next_victim++) {
      if (my_attackers & b.bitboard[turn + next_victim]) break;
    }

    // remove this attacker from occupied:
    occupied ^= (1L << LSB(my_attackers & b.bitboard[turn + next_victim]));

    if (next_victim == PAWN || next_victim == BISHOP || next_victim == QUEEN) {
      attackers |= diag_moves_magic(to, occupied) & bishops;
    }

    if (next_victim == ROOK || next_victim == QUEEN) {
      attackers |= line_moves_magic(to, occupied) & rooks;
    }

    attackers &= occupied;
    turn = (turn == WHITE) ? BLACK : WHITE;

    // negamax the balance score and add the score of the next victim:
    balance = -balance + SEE_PIECE_VALUES[previous_attacker];
    previous_attacker = next_victim;

    // if we're doing better than our opponent, we can break as well:
    if (balance >= 0) break;
  } while (true);

  // there might be one last capture available:
  my_attackers = attackers & (turn == WHITE ? b.W : b.B);
  if (my_attackers) {
    balance = -balance + SEE_PIECE_VALUES[previous_attacker];
    turn = (turn == WHITE) ? BLACK : WHITE;
  }

  // return the balance with respect to the player whose turn it is:
  return (b.turn == turn) ? -balance : balance;
}

// estimated_move_value(): roughly estimate the move's value.
// code is straight from andrew grant's ethereal engine
int estimated_move_value(int move) {
  int value = SEE_PIECE_VALUES[b.piece_board[MOVE_TO(move)]];

  if (MOVE_IS_PROMOTION(move)) {
    value += SEE_PIECE_VALUES[MOVE_PROMOTION_PIECE(move)] - SEE_PIECE_VALUES[PAWN];
  }

  else if (MOVE_IS_EP(move)) value = SEE_PIECE_VALUES[PAWN];

  else if (MOVE_IS_CASTLE(move)) value = 0;

  return value;
} */
