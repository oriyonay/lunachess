#include "board.h"

// the board constructor, which parses a FEN-string:
board::board(char* FEN) : material(0), num_moves_played(0), castle_rights(0),
  CANT_CAPTURE(0L), CAN_CAPTURE(0L), EMPTY_SQUARES(0L), CAN_MOVE_TO(0L),
  OCCUPIED_SQUARES(0L) {
  // clear the bitboards:
  memset(bitboard, 0, 12 * sizeof(U64));

  // clear the piece board:
  // NOTE: the piece board is used for quick piece-square lookup
  memset(piece_board, 12, 64 * sizeof(int));

  // clear the move history list:
  memset(move_history, 0, MAX_GAME_MOVES * sizeof(int));

  // parse the main FEN. (i is the shift index)
  int i = 0;
  while (*FEN != ' ') {
    if (isdigit(*FEN)) i += (*FEN) - '0' - 1; // -1 to cancel out i++
    else if (*FEN != '/') {
      bitboard[PIECE_INDICES.find(*FEN)->second] |= (1L << i);
      piece_board[i] = PIECE_INDICES.find(*FEN)->second;
      material += MATERIAL.find(*FEN)->second;
    }
    else i--; // to handle '/' (we -1 to cancel out the i++)

    FEN++;
    i++;
  }

  assert(i == 64);

  // parse the turn value:
  FEN++;
  turn = ((*FEN) == 'w') ? WHITE : BLACK;

  // parse castling rights:
  FEN += 2;
  while (*FEN != ' ') {
    switch (*FEN) {
      case 'K':
        castle_rights |= (1 << 3);
        break;
      case 'Q':
        castle_rights |= (1 << 2);
        break;
      case 'k':
        castle_rights |= (1 << 1);
        break;
      case 'q':
        castle_rights |= (1 << 0);
        break;
    }
    FEN++;
  }

  // update move information bitboards:
  update_move_info_bitboards();

  // parse the rest of the FEN-string: TODO
}

/* get_moves(): generates and returns a list of all possible PSEUDO-LEGAL moves
 * on the board. a reference to an integer num_moves is taken, and is updated
 * with the number of moves generated.
 *
 * moves are represented as ints, with the following bit representation:
 * 1111 1100 0000 0000 0000 0000 0000 0000 -> TO-square (6 bits)
 * 0000 0011 1111 0000 0000 0000 0000 0000 -> FROM-square (6 bits)
 * 0000 0000 0000 1111 0000 0000 0000 0000 -> piece captured (4 bits)
 * 0000 0000 0000 0000 1000 0000 0000 0000 -> is this an en-passant capture? (1 bit)
 * 0000 0000 0000 0000 0100 0000 0000 0000 -> is this a pawn's first move? (1 bit)
 * 0000 0000 0000 0000 0010 0000 0000 0000 -> is this a castle? (1 bit)
 * 0000 0000 0000 0000 0001 0000 0000 0000 -> is this a promotion? (1 bit)
 * 0000 0000 0000 0000 0000 1111 0000 0000 -> if so, to what piece? (4 bits)
 * 0000 0000 0000 0000 0000 0000 1111 0000 -> previous castle rights (4 bits)
 * 0000 0000 0000 0000 0000 0000 0000 1111 -> piece moved, i.e., a white knight (4 bits)
*/
int* board::get_moves(int& num_moves) {
  int* move_list = new int[MAX_POSITION_MOVES];
  num_moves = 0;

  // update move generation bitboards:
  update_move_info_bitboards();

  add_pawn_moves(move_list, num_moves);
  add_diag_moves(move_list, num_moves);
  add_line_moves(move_list, num_moves);
  add_knight_moves(move_list, num_moves);
  add_king_moves(move_list, num_moves);

  return move_list;
}

// make_move(char* move): converts the given human-formatted (i.e., e4e5) move
// to our machine-formatted (int) move, and makes the move.
bool board::make_move(char* move) {
  // get TO and FROM coordinates:
  int from = (*move) - 'a';
  move++;
  from += 8 * (7 - ((*move) - '1'));
  move++;

  int to = (*move) - 'a';
  move++;
  to += 8 * (7 - ((*move) - '1'));
  move++;

  // if this is a promotion, to what piece?
  int promotion_piece = NONE;
  switch (*move) {
    case '\0': break; // default - no promotion
    case 'q':
      promotion_piece = turn + QUEEN;
      break;
    case 'r':
      promotion_piece = turn + ROOK;
      break;
    case 'b':
      promotion_piece = turn + BISHOP;
      break;
    case 'n':
      promotion_piece = turn + KNIGHT;
      break;
  }

  // now we need to find a matching move-int in our move-list
  int num_moves;
  int* moves = get_moves(num_moves);
  bool found = false;
  int m;
  for (int i = 0; i < num_moves; i++) {
    if ((MOVE_TO(moves[i]) == to) &&
        (MOVE_FROM(moves[i]) == from) &&
        (MOVE_PROMOTION_PIECE(moves[i]) == promotion_piece)) {
          m = moves[i];
          found = true;
          break;
    }
  }

  if (!found) return false;
  delete[] moves;

  return make_move(m);
}

// make_move(): makes the given (presumably pseudo-legal) move and checks if
// it's legal. returns true if legal, false otherwise.
bool board::make_move(int move) {
  // first of all, let's push the move to our move_history stack:
  move_history[num_moves_played++] = move;

  // get move info:
  int to = MOVE_TO(move);
  int from = MOVE_FROM(move);
  int captured = MOVE_CAPTURED(move);
  int piece_moved = MOVE_PIECEMOVED(move);
  int ep = MOVE_IS_EP(move);
  int castle = MOVE_IS_CASTLE(move);
  int promotion = MOVE_IS_PROMOTION(move);

  // move the piece to its new board location:
  bitboard[piece_moved] |= (1L << to);
  piece_board[to] = piece_moved;

  // remove the piece from its old location:
  bitboard[piece_moved] ^= (1L << from);
  piece_board[from] = NONE;

  // update castle rights (in case a piece took one of the rooks):
  switch(to) {
    case 63:
      castle_rights &= 0x7;
      break;
    case 56:
      castle_rights &= 0xB;
      break;
    case 7:
      castle_rights &= 0xD;
      break;
    case 0:
      castle_rights &= 0xE;
      break;
  }

  // update castle rights (in case either the king or one of the rooks moved):
  if (turn == WHITE) {
    if (!(bitboard[WK] & INIT_WK)) castle_rights &= 0x3;
    if ((from == 63 || to == 63) && CAN_CWK(castle_rights)) castle_rights &= 0x7;
    if ((from == 56 || to == 56) && CAN_CWQ(castle_rights)) castle_rights &= 0xB;
  }
  else {
    if (!(bitboard[BK] & INIT_BK)) castle_rights &= 0xC;
    if ((from == 7 || to == 7) && CAN_CBK(castle_rights)) castle_rights &= 0xD;
    if ((from == 0 || to == 0) && CAN_CBQ(castle_rights)) castle_rights &= 0xE;
  }

  // if a piece was captured, update the board and material:
  if (captured != NONE) {
    bitboard[captured] ^= (1L << to);

    material -= PIECE_TO_MATERIAL[captured];
  }

  // if this was an en passant, remove the piece from the board:
  if (ep) {
    // then we flipped the wrong bit earlier. let's flip it again:
    bitboard[captured] ^= (1L << to);

    // in which direction was the capture?
    switch (from - to) {
      case 7:
        // white en passant captures right
        bitboard[BP] ^= (1L << (from + 1));
        piece_board[from + 1] = NONE;
        break;
      case 9:
        // white en passant captures left
        bitboard[BP] ^= (1L << (from - 1));
        piece_board[from - 1] = NONE;
        break;
      case -9:
        // black en passant captures right
        bitboard[WP] ^= (1L << (from + 1));
        piece_board[from + 1] = NONE;
        break;
      case -7:
        // black en passant captures left
        bitboard[WP] ^= (1L << (from - 1));
        piece_board[from - 1] = NONE;
        break;
    }
  }

  if (castle) {
    // move the rook:
    switch (to) {
      case 62:
        // white kingside castle
        bitboard[WR] ^= 0xA000000000000000L; // (1 << 61) | (1 << 63) to flip both bits at once
        piece_board[61] = WR;
        piece_board[63] = NONE;
        castle_rights &= 0x3; // cancel out castle rights
        break;
      case 58:
        // white queenside castle
        bitboard[WR] ^= 0x900000000000000L; // (1 << 56) | (1 << 59) to flip both bits at once
        piece_board[59] = WR;
        piece_board[56] = NONE;
        castle_rights &= 0x3; // cancel out castle rights
        break;
      case 6:
        // black kingside castle
        bitboard[BR] ^= 0xA0L; // (1 << 5) | (1 << 7) to flip both bits at once
        piece_board[5] = BR;
        piece_board[7] = NONE;
        castle_rights &= 0xC; // cancel out castle rights
        break;
      case 2:
        // black queenside castle
        bitboard[BR] ^= 0x9L; // (1 << 0) | (1 << 3) to flip both bits at once
        piece_board[3] = BR;
        piece_board[0] = NONE;
        castle_rights &= 0xC; // cancel out castle rights
        break;
    }
  }

  if (promotion) {
    // to which piece are we promoting?
    int promoted_piece = (move >> 8) & 0xF;

    // we need to replace the piece we previously moved TO with the new promoted piece:
    bitboard[piece_moved] ^= (1L << to);
    bitboard[promoted_piece] |= (1L << to);
    piece_board[to] = promoted_piece;

    // update material:
    material += PIECE_TO_MATERIAL[promoted_piece] - PIECE_TO_MATERIAL[piece_moved];
  }

  // now comes the moment of truth. is this move legal? well, only if we're not in check:
  update_move_info_bitboards();

  // flip the turn:
  turn = (turn == WHITE) ? BLACK : WHITE;

  return !(UNSAFE & bitboard[KING + ((turn == WHITE) ? BLACK : WHITE)]);
}

// undo_move(): undoes the last move made
void board::undo_move() {
  // first of all, let's pop the move off our move_history stack:
  num_moves_played--;

  int move = move_history[num_moves_played];

  // get move info:
  int to = MOVE_TO(move);
  int from = MOVE_FROM(move);
  int captured = MOVE_CAPTURED(move);
  int piece_moved = MOVE_PIECEMOVED(move);
  int ep = MOVE_IS_EP(move);
  int castle = MOVE_IS_CASTLE(move);
  int promotion = MOVE_IS_PROMOTION(move);
  int pcr = MOVE_PCR(move);

  // remove the piece back from its current board location:
  bitboard[piece_moved] ^= (1L << to);
  piece_board[to] = NONE;

  // put the piece back in its previous location:
  bitboard[piece_moved] |= (1L << from);
  piece_board[from] = piece_moved;

  // reload castle rights:
  castle_rights = pcr;

  // if a piece was captured, update the board and material:
  if (captured != NONE) {
    bitboard[captured] |= (1L << to);
    piece_board[to] = captured;

    material += PIECE_TO_MATERIAL[captured];
  }

  // if this was an en passant, put the pawn back on the board:
  if (ep) {
    // then we flipped the wrong bit in the 'captured' section and need to flip it again:
    bitboard[captured] ^= (1L << to);

    // in which direction was the capture?
    switch (from - to) {
      case 7:
        // white en passant captures right
        bitboard[BP] ^= (1L << (from + 1));
        piece_board[from - 7] = NONE;
        piece_board[from + 1] = BP;
        break;
      case 9:
        // white en passant captures left
        bitboard[BP] ^= (1L << (from - 1));
        piece_board[from - 9] = NONE;
        piece_board[from - 1] = BP;
        break;
      case -9:
        // black en passant captures right
        bitboard[WP] ^= (1L << (from + 1));
        piece_board[from + 9] = NONE;
        piece_board[from + 1] = WP;
        break;
      case -7:
        // black en passant captures left
        bitboard[WP] ^= (1L << (from - 1));
        piece_board[from + 7] = NONE;
        piece_board[from - 1] = WP;
        break;
    }
  }

  if (castle) {
    // move the rook:
    switch (to) {
      case 62:
        // white kingside castle
        bitboard[WR] ^= 0xA000000000000000L; // (1 << 61) | (1 << 63) to flip both bits at once
        piece_board[63] = WR;
        piece_board[61] = NONE;
        break;
      case 58:
        // white queenside castle
        bitboard[WR] ^= 0x900000000000000L; // (1 << 56) | (1 << 59) to flip both bits at once
        piece_board[56] = WR;
        piece_board[59] = NONE;
        break;
      case 6:
        // black kingside castle
        bitboard[BR] ^= 0xA0L; // (1 << 5) | (1 << 7) to flip both bits at once
        piece_board[7] = BR;
        piece_board[5] = NONE;
        break;
      case 2:
        // black queenside castle
        bitboard[BR] ^= 0x9L; // (1 << 0) | (1 << 3) to flip both bits at once
        piece_board[0] = BR;
        piece_board[3] = NONE;
        break;
    }
  }

  if (promotion) {
    // to which piece are we promoting?
    int promoted_piece = (move >> 8) & 0xF;

    // we need to remove the new piece:
    bitboard[piece_moved] ^= (1L << to);
    bitboard[promoted_piece] ^= (1L << to);
    piece_board[to] = captured;

    // update material:
    material -= PIECE_TO_MATERIAL[promoted_piece] - PIECE_TO_MATERIAL[piece_moved];
  }

  // flip the turn:
  turn = (turn == WHITE) ? BLACK : WHITE;
}

// print(): prints the board:
void board::print() {
  for (int i = 0; i < 64; i++) {
    bool on_board = false;
    for (int j = 0; j < 12; j++) {
      if ((bitboard[j] >> i) & 1) {
        printf("%c ", PIECE_CHARS[j]);
        on_board = true;
        break;
      }
    }
    if (!on_board) printf(". ");
    if (i % 8 == 7) printf("\n");
  }

  printf("\nturn: %c\n", (turn == WHITE) ? 'W' : 'B');
  printf("material: %d\n", material);
}

// add all possible pawn moves to the stack:
void board::add_pawn_moves(int* move_list, int& num_moves) {
  U64 moves;
  char idx;
  if (turn == WHITE) {
    // ----- look for captures in both directions: -----
    // every 1 in this bitboard corresponds to a piece that can be captured:
    moves = (bitboard[WP] >> 7) & CAN_CAPTURE & ~RANKS[8] & ~FILES[A]; // capture right

    // bitscan to find these captures:
    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      // the captured piece is piece_board[idx]
      move_list[num_moves++] = move_int(idx, idx+7, piece_board[idx], 0, 0, 0,
                                        0, NONE, castle_rights, WP);
    }

    moves = (bitboard[WP] >> 9) & CAN_CAPTURE & ~RANKS[8] & ~FILES[H];  // capture left

    // bitscan to find these captures:
    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx+9, piece_board[idx], 0, 0, 0,
                                        0, NONE, castle_rights, WP);
    }

    // ----- look for 1-square pawn push moves: -----
    // every 1 in this bitboard corresponds to a square that a pawn can be pushed
    // a single square forward to:
    moves = (bitboard[WP] >> 8) & EMPTY_SQUARES & ~RANKS[8];

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx+8, NONE, 0, 0, 0,
                                        0, NONE, castle_rights, WP);
    }

    // ----- look for 2-square pawn push moves: -----
    // every 1 in this bitboard corresponds to a 2-square pawn push:
    moves = (bitboard[WP] >> 16) & EMPTY_SQUARES & (EMPTY_SQUARES >> 8) & RANKS[4];

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx+16, NONE, 0, 1, 0,
                                        0, NONE, castle_rights, WP);
    }

    // ----- look for pawn promotion by push: -----
    moves = (bitboard[WP] >> 8) & EMPTY_SQUARES & RANKS[8];

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx+8, NONE, 0, 0, 0,
                                        1, WQ, castle_rights, WP);
      move_list[num_moves++] = move_int(idx, idx+8, NONE, 0, 0, 0,
                                        1, WN, castle_rights, WP);
      move_list[num_moves++] = move_int(idx, idx+8, NONE, 0, 0, 0,
                                        1, WR, castle_rights, WP);
      move_list[num_moves++] = move_int(idx, idx+8, NONE, 0, 0, 0,
                                        1, WB, castle_rights, WP);
    }

    // ----- look for pawn promotion by capture: -----
    moves = (bitboard[WP] >> 7) & CAN_CAPTURE & RANKS[8] & ~FILES[A]; // capture right

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx+7, piece_board[idx], 0, 0, 0,
                                        1, WQ, castle_rights, WP);
      move_list[num_moves++] = move_int(idx, idx+7, piece_board[idx], 0, 0, 0,
                                        1, WN, castle_rights, WP);
      move_list[num_moves++] = move_int(idx, idx+7, piece_board[idx], 0, 0, 0,
                                        1, WR, castle_rights, WP);
      move_list[num_moves++] = move_int(idx, idx+7, piece_board[idx], 0, 0, 0,
                                        1, WB, castle_rights, WP);
    }

    moves = (bitboard[WP] >> 9) & CAN_CAPTURE & RANKS[8] & ~FILES[H]; // capture left

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx+9, piece_board[idx], 0, 0, 0,
                                        1, WQ, castle_rights, WP);
      move_list[num_moves++] = move_int(idx, idx+9, piece_board[idx], 0, 0, 0,
                                        1, WN, castle_rights, WP);
      move_list[num_moves++] = move_int(idx, idx+9, piece_board[idx], 0, 0, 0,
                                        1, WR, castle_rights, WP);
      move_list[num_moves++] = move_int(idx, idx+9, piece_board[idx], 0, 0, 0,
                                        1, WB, castle_rights, WP);
    }

    // ----- look for a possible en passant capture: -----
    if (MOVE_IS_PAWNFIRST(move_history[num_moves_played-1])) {
      char ep_file = FILE_NO(MOVE_TO(move_history[num_moves_played-1]));
      // look for en passant to the right:
      moves = (bitboard[WP] << 1) & bitboard[BP] & RANKS[5] & ~FILES[A] & FILES[ep_file];
      if (moves) {
        idx = LSB(moves);

        move_list[num_moves++] = move_int(idx-8, idx-1, BP, 1, 0, 0,
                                          0, NONE, castle_rights, WP);
      }

      // look for en passant to the left:
      moves = (bitboard[WP] >> 1) & bitboard[BP] & RANKS[5] & ~FILES[H] & FILES[ep_file];
      if (moves) {
        idx = LSB(moves);

        move_list[num_moves++] = move_int(idx-8, idx+1, BP, 1, 0, 0,
                                          0, NONE, castle_rights, WP);
      }
    }
  }

  else {
    // ----- look for captures in both directions: -----
    // every 1 in this bitboard corresponds to a piece that can be captured:
    moves = (bitboard[BP] << 7) & CAN_CAPTURE & ~RANKS[1] & ~FILES[H]; // capture left

    // bitscan to find these captures:
    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx-7, piece_board[idx], 0, 0, 0,
                                        0, NONE, castle_rights, BP);
    }

    moves = (bitboard[BP] << 9) & CAN_CAPTURE & ~RANKS[1] & ~FILES[A];  // capture right

    // bitscan to find these captures:
    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx-9, piece_board[idx], 0, 0, 0,
                                        0, NONE, castle_rights, BP);
    }

    // ----- look for 1-square pawn push moves: -----
    // every 1 in this bitboard corresponds to a square that a pawn can be pushed
    // a single square forward to:
    moves = (bitboard[BP] << 8) & EMPTY_SQUARES & ~RANKS[1];

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx-8, NONE, 0, 0, 0,
                                        0, NONE, castle_rights, BP);
    }

    // ----- look for 2-square pawn push moves: -----
    // every 1 in this bitboard corresponds to a 2-square pawn push:
    moves = (bitboard[BP] << 16) & EMPTY_SQUARES & (EMPTY_SQUARES << 8) & RANKS[5];

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx-16, NONE, 0, 1, 0,
                                      0, NONE, castle_rights, BP);
    }

    // ----- look for pawn promotion by push: -----
    moves = (bitboard[BP] << 8) & EMPTY_SQUARES & RANKS[1];

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx-8, NONE, 0, 0, 0,
                                        1, BQ, castle_rights, BP);
      move_list[num_moves++] = move_int(idx, idx-8, NONE, 0, 0, 0,
                                        1, BN, castle_rights, BP);
      move_list[num_moves++] = move_int(idx, idx-8, NONE, 0, 0, 0,
                                        1, BR, castle_rights, BP);
      move_list[num_moves++] = move_int(idx, idx-8, NONE, 0, 0, 0,
                                        1, BB, castle_rights, BP);
    }

    // ----- look for pawn promotion by capture: -----
    moves = ((bitboard[BP] << 7) & CAN_CAPTURE & RANKS[1] & ~FILES[H]); // capture left

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx-7, piece_board[idx], 0, 0, 0,
                                        1, BQ, castle_rights, BP);
      move_list[num_moves++] = move_int(idx, idx-7, piece_board[idx], 0, 0, 0,
                                        1, BN, castle_rights, BP);
      move_list[num_moves++] = move_int(idx, idx-7, piece_board[idx], 0, 0, 0,
                                        1, BR, castle_rights, BP);
      move_list[num_moves++] = move_int(idx, idx-7, piece_board[idx], 0, 0, 0,
                                        1, BB, castle_rights, BP);
    }

    moves = (bitboard[BP] << 9) & CAN_CAPTURE & RANKS[1] & ~FILES[A]; // capture right

    while (moves) {
      idx = LSB(moves);
      POP_LSB(moves);

      move_list[num_moves++] = move_int(idx, idx-9, piece_board[idx], 0, 0, 0,
                                        1, BQ, castle_rights, BP);
      move_list[num_moves++] = move_int(idx, idx-9, piece_board[idx], 0, 0, 0,
                                        1, BN, castle_rights, BP);
      move_list[num_moves++] = move_int(idx, idx-9, piece_board[idx], 0, 0, 0,
                                        1, BR, castle_rights, BP);
      move_list[num_moves++] = move_int(idx, idx-9, piece_board[idx], 0, 0, 0,
                                        1, BB, castle_rights, BP);
    }

    // ----- look for a possible en passant capture: -----
    if (MOVE_IS_PAWNFIRST(move_history[num_moves_played-1])) {
      char ep_file = FILE_NO(MOVE_TO(move_history[num_moves_played-1]));

      // look for en passant capture to the right:
      moves = (bitboard[BP] << 1) & bitboard[WP] & RANKS[4] & ~FILES[A] & FILES[ep_file];
      if (moves) {
        idx = LSB(moves);

        move_list[num_moves++] = move_int(idx+8, idx-1, WP, 1, 0, 0,
                                          0, NONE, castle_rights, BP);
      }

      // look for en passant capture to the left:
      moves = (bitboard[BP] >> 1) & bitboard[WP] & RANKS[4] & ~FILES[H] & FILES[ep_file];
      if (moves) {
        idx = LSB(moves);

        move_list[num_moves++] = move_int(idx+8, idx+1, WP, 1, 0, 0,
                                          0, NONE, castle_rights, BP);
      }
    }
  }
}

// add all possible bishop-like moves to the stack:
void board::add_diag_moves(int* move_list, int& num_moves) {
  // we include the queen in the 'bishop bitboard' and calculate queen moves as well:
  U64 bishop_bitboard = (turn == WHITE) ?
    bitboard[WB] | bitboard[WQ] :
    bitboard[BB] | bitboard[BQ];

  // in this nested loop, i loops through every individual bishop on the board,
  // and j loops through every possible location that bishop can travel to.
  // idx and idx2 represent the beginning and ending square indices of each move, respectively
  U64 i = ISOLATE_LSB(bishop_bitboard);
  U64 j;
  U64 possible;
  char idx;
  char idx2;

  while (i) {
    idx = LSB(i);
    possible = diag_moves(idx, OCCUPIED_SQUARES) & CAN_MOVE_TO;

    j = ISOLATE_LSB(possible);
    while (j) {
      idx2 = LSB(j);

      // add the move to the move list. was a piece captured?
      if (OCCUPIED_SQUARES & j) {
        move_list[num_moves++] = move_int(idx2, idx, piece_board[idx2], 0, 0, 0, 0,
                                          NONE, castle_rights, piece_board[idx]);
      }
      else {
        move_list[num_moves++] = move_int(idx2, idx, NONE, 0, 0, 0, 0,
                                          NONE, castle_rights, piece_board[idx]);
      }

      // remove this position from possible (equivalent to POP_LSB macro but we already have j)
      possible &= ~j;
      j = ISOLATE_LSB(possible);
    }

    bishop_bitboard &= ~i;
    i = ISOLATE_LSB(bishop_bitboard);
  }
}

// add all possible rook-like moves to the stack:
void board::add_line_moves(int* move_list, int& num_moves) {
  // we include the queen in the 'rook bitboard' and calculate queen moves as well:
  U64 rook_bitboard = (turn == WHITE) ?
    bitboard[WR] | bitboard[WQ] :
    bitboard[BR] | bitboard[BQ];

  // in this nested loop, i loops through every individual rook on the board,
  // and j loops through every possible location that rook can travel to.
  // idx and idx2 represent the beginning and ending square indices of each move, respectively
  U64 i = ISOLATE_LSB(rook_bitboard);
  U64 j;
  U64 possible;
  char idx;
  char idx2;

  while (i) {
    idx = LSB(i);
    possible = line_moves(idx, OCCUPIED_SQUARES) & CAN_MOVE_TO;

    j = ISOLATE_LSB(possible);
    while (j) {
      idx2 = LSB(j);

      // add the move to the move list. was a piece captured?
      if (OCCUPIED_SQUARES & j) {
        move_list[num_moves++] = move_int(idx2, idx, piece_board[idx2], 0, 0, 0, 0,
                                          NONE, castle_rights, piece_board[idx]);
      }
      else {
        move_list[num_moves++] = move_int(idx2, idx, NONE, 0, 0, 0, 0,
                                          NONE, castle_rights, piece_board[idx]);
      }

      // remove this position from possible (equivalent to POP_LSB macro but we already have j)
      possible &= ~j;
      j = ISOLATE_LSB(possible);
    }

    rook_bitboard &= ~i;
    i = ISOLATE_LSB(rook_bitboard);
  }
}

// add all possible knight moves to the stack:
void board::add_knight_moves(int* move_list, int& num_moves) {
  U64 knight_bitboard = (turn == WHITE) ? bitboard[WN] : bitboard[BN];

  // in this nested loop, i loops through every individual knight on the board,
  // and j loops through every possible location that knight can travel to.
  // idx and idx2 represent the beginning and ending square indices of each move, respectively
  U64 i = ISOLATE_LSB(knight_bitboard);
  U64 j;
  U64 possible;
  char idx;
  char idx2;

  while (i) {
    idx = LSB(i);

    // figure out how to shift the knight span bitboard to get possible moves:
    possible = (idx > 18) ? KNIGHT_SPAN << (idx - 18) : KNIGHT_SPAN >> (18 - idx);
    possible &= (idx % 8 < 4) ? ~FILE_GH & CAN_MOVE_TO : ~FILE_AB & CAN_MOVE_TO;

    j = ISOLATE_LSB(possible);
    while (j) {
      idx2 = LSB(j);

      // add the move to the move list. was a piece captured?
      if (OCCUPIED_SQUARES & j) {
        move_list[num_moves++] = move_int(idx2, idx, piece_board[idx2], 0, 0, 0, 0,
                                          NONE, castle_rights, KNIGHT + turn);
      }
      else {
        move_list[num_moves++] = move_int(idx2, idx, NONE, 0, 0, 0, 0,
                                          NONE, castle_rights, KNIGHT + turn);
      }

      // remove this position from possible (equivalent to POP_LSB macro but we already have j)
      possible &= ~j;
      j = ISOLATE_LSB(possible);
    }

    knight_bitboard &= ~i;
    i = ISOLATE_LSB(knight_bitboard);
  }
}

// add all possible knight moves to the stack:
void board::add_king_moves(int* move_list, int& num_moves) {
  U64 king_bitboard = (turn == WHITE) ? bitboard[WK] : bitboard[BK];
  char idx = LSB(king_bitboard);
  char idx2;

  // figure out how to shift the king span bitboard to get possible moves:
  U64 possible = (idx > 9) ? KING_SPAN << (idx - 9) : KING_SPAN >> (9 - idx);
  possible &= (idx % 8 < 4) ? ~FILE_GH & CAN_MOVE_TO : ~FILE_AB & CAN_MOVE_TO;

  U64 j = ISOLATE_LSB(possible);
  while (j) {
    idx2 = LSB(j);

    // add the move to the move list. was a piece captured?
    if (OCCUPIED_SQUARES & j) {
      move_list[num_moves++] = move_int(idx2, idx, piece_board[idx2], 0, 0, 0, 0,
                                        NONE, castle_rights, KING + turn);
    }
    else {
      move_list[num_moves++] = move_int(idx2, idx, NONE, 0, 0, 0, 0,
                                        NONE, castle_rights, KING + turn);
    }

    // remove this position from possible (equivalent to POP_LSB macro but we already have j)
    possible &= ~j;
    j = ISOLATE_LSB(possible);
  }

  // get castling moves:
  if (!(UNSAFE & bitboard[KING + turn])) {
    if (turn == WHITE) {
      // check for kingside castle:
      if (CAN_CWK(castle_rights) && !((OCCUPIED_SQUARES | UNSAFE) & CWK_SAFE_SPACES)) {
        move_list[num_moves++] = move_int(62, 60, NONE, 0, 0, 1, 0,
                                          NONE, castle_rights, WK);
      }

      // check for queenside castle:
      if (CAN_CWQ(castle_rights) && !((UNSAFE & CWQ_SAFE_SPACES) || (OCCUPIED_SQUARES & CWQ_EMPTY_SPACES))) {
        move_list[num_moves++] = move_int(58, 60, NONE, 0, 0, 1, 0,
                                          NONE, castle_rights, WK);
      }
    }

    else {
      // check for kingside castle:
      if (CAN_CBK(castle_rights) && !((OCCUPIED_SQUARES | UNSAFE) & CBK_SAFE_SPACES)) {
        move_list[num_moves++] = move_int(6, 4, NONE, 0, 0, 1, 0,
                                          NONE, castle_rights, BK);
      }

      // check for queenside castle:
      if (CAN_CBQ(castle_rights) && !((UNSAFE & CBQ_SAFE_SPACES) || (OCCUPIED_SQUARES & CBQ_EMPTY_SPACES))) {
        move_list[num_moves++] = move_int(2, 4, NONE, 0, 0, 1, 0,
                                          NONE, castle_rights, BK);
      }
    }
  }
}

inline void board::update_move_info_bitboards() {
  if (turn == WHITE) {
    CANT_CAPTURE = bitboard[WP] | bitboard[WN] | bitboard[WB] | bitboard[WR] |
                   bitboard[WQ] | bitboard[WK] | bitboard[BK];
    CAN_CAPTURE  = bitboard[BP] | bitboard[BN] | bitboard[BB] | bitboard[BR] |
                   bitboard[BQ];
  }
  else {
    CANT_CAPTURE = bitboard[BP] | bitboard[BN] | bitboard[BB] | bitboard[BR] |
                   bitboard[BQ] | bitboard[BK] | bitboard[WK];
    CAN_CAPTURE  = bitboard[WP] | bitboard[WN] | bitboard[WB] | bitboard[WR] |
                   bitboard[WQ];
  }

  CAN_MOVE_TO = ~CANT_CAPTURE;
  OCCUPIED_SQUARES = CAN_CAPTURE | CANT_CAPTURE;
  EMPTY_SQUARES = ~OCCUPIED_SQUARES;

  update_unsafe();
}

void board::update_unsafe() {
  int NOT_TURN = (turn == WHITE) ? BLACK : WHITE;

  U64 opp_knight = bitboard[KNIGHT + NOT_TURN];
  U64 opp_king = bitboard[KING + NOT_TURN];
  U64 opp_QB = bitboard[QUEEN + NOT_TURN] | bitboard[BISHOP + NOT_TURN];
  U64 opp_QR = bitboard[QUEEN + NOT_TURN] | bitboard[ROOK + NOT_TURN];

  // pawn attacks:
  if (turn == WHITE) {
    UNSAFE  = ((bitboard[PAWN + NOT_TURN] << 7) & ~FILES[H]);
    UNSAFE |= ((bitboard[PAWN + NOT_TURN] << 9) & ~FILES[A]);
  }
  else {
    UNSAFE  = ((bitboard[PAWN + NOT_TURN] >> 7) & ~FILES[A]);
    UNSAFE |= ((bitboard[PAWN + NOT_TURN] >> 9) & ~FILES[H]);
  }

  // knight attacks:
  U64 possible;
  char idx;
  U64 i = ISOLATE_LSB(opp_knight);

  while (i) {
    idx = LSB(i);
    possible  = (idx > 18) ? KNIGHT_SPAN << (idx - 18) : KNIGHT_SPAN >> (18 - idx);
    possible &= (idx % 8 < 4) ? ~FILE_GH : ~FILE_AB;
    UNSAFE |= possible;

    opp_knight &= ~i;
    i = ISOLATE_LSB(opp_knight);
  }

  // queen / bishop attacks diagonally:
  i = ISOLATE_LSB(opp_QB);
  while (i) {
    idx = LSB(i);
    possible = diag_moves(idx, OCCUPIED_SQUARES);
    UNSAFE |= possible;

    opp_QB &= ~i;
    i = ISOLATE_LSB(opp_QB);
  }

  // queen / rook attacks horizontally / vertically:
  i = ISOLATE_LSB(opp_QR);
  while (i) {
    idx = LSB(i);
    possible = line_moves(idx, OCCUPIED_SQUARES);
    UNSAFE |= possible;

    opp_QR &= ~i;
    i = ISOLATE_LSB(opp_QR);
  }

  // king attacks:
  idx = LSB(opp_king);
  possible = (idx > 9) ? KING_SPAN << (idx - 9) : KING_SPAN >> (9 - idx);
  possible &= (idx % 8 < 4) ? ~FILE_GH : ~FILE_AB;
  UNSAFE |= possible;
}

// is_check(): ASSUMES UNSAFE BITBOARD HAS BEEN UPDATED (which it is if we've
// called get_moves())
bool board::is_check() {
  return UNSAFE & bitboard[KING + turn];
}

/* ---------- BOARD UTILITY FUNCTIONS ---------- */

// generates a move integer:
inline U64 move_int(char TO, char FROM, char CAPTURED, char EP, char PF, char CASTLE,
                    char PROM, char PROM_PIECE, char PCR, char PM) {
  return (TO << 26) | (FROM << 20) | (CAPTURED << 16) | (EP << 15) | (PF << 14) |
         (CASTLE << 13) | (PROM << 12) | (PROM_PIECE << 8) | (PCR << 4) | PM;
}

// generate all possible horizontal and vertical (rook-like) moves.
// here, s = the board index of the slider piece location and
// OCCUPIED = bitboard of the occupied pieces
inline U64 line_moves(char s, U64 OCCUPIED) {
  U64 s_bin = 1L << s;
  U64 horizontal = (OCCUPIED - (2 * s_bin)) ^
      reverse_bits(reverse_bits(OCCUPIED) - (2 * reverse_bits(s_bin)));
  U64 vertical = ((OCCUPIED & FILES[s % 8]) - (2 * s_bin)) ^
      reverse_bits(reverse_bits(OCCUPIED & FILES[s % 8]) - (2 * reverse_bits(s_bin)));

  return (horizontal & RANKS[8 - (s / 8)]) | (vertical & FILES[s % 8]);
}

// generate all possible diagonal (bishop-like) moves:
inline U64 diag_moves(char s, U64 OCCUPIED) {
  U64 s_bin = 1L << s;
  U64 diag = DIAGONAL_MASKS[(s / 8) + (s % 8)];
  U64 antidiag = ANTIDIAGONAL_MASKS[(s / 8) + 7 - (s % 8)];

  diag = (((OCCUPIED & diag) - (2 * s_bin)) ^
         reverse_bits(reverse_bits(OCCUPIED & diag) -
         (2 * reverse_bits(s_bin)))) & diag;
  antidiag = (((OCCUPIED & antidiag) - (2 * s_bin)) ^
         reverse_bits(reverse_bits(OCCUPIED & antidiag) -
         (2 * reverse_bits(s_bin)))) & antidiag;

  return diag | antidiag;
}

// reverse_bits(n): reverses the bits of a U64
inline U64 reverse_bits(U64 n) {
    // swap odd and even bits
    n = ((n >> 1) & 0x5555555555555555) | ((n & 0x5555555555555555) << 1);
    // swap consecutive pairs
    n = ((n >> 2) & 0x3333333333333333) | ((n & 0x3333333333333333) << 2);
    // swap nibbles ...
    n = ((n >> 4) & 0x0F0F0F0F0F0F0F0F) | ((n & 0x0F0F0F0F0F0F0F0F) << 4);
    // swap bytes
    n = ((n >> 8) & 0x00FF00FF00FF00FF) | ((n & 0x00FF00FF00FF00FF) << 8);
    // swap 2-byte long pairs
    n = ( n >> 16 & 0x0000FFFF0000FFFF) | ((n & 0x0000FFFF0000FFFF) << 16);
    // swap 4-byte long pairs
    n = ( n >> 32                     ) | ( n                       << 32);
    return n;
}
