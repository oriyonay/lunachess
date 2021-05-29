#include "engine.h"

// TODO: remove this
void pm(int m) {
  int to = (m >> 26) & 0x3F;
  int from = (m >> 20) & 0x3F;

  char* move_str = new char[6];
  move_str[5] = '\0';
  move_str[4] = '\0'; // TODO: REMOVE THIS

  move_str[0] = ((from % 8) + 'a');
  move_str[1] = (8 - (from / 8) + '0');
  move_str[2] = ((to % 8) + 'a');
  move_str[3] = (8 - (to / 8) + '0');

  printf("%s\n", move_str);
}

// todo: remove this
int validate_board(board* b) {
  // error 1: is there more than 1 piece on a single square in the bitboards?
  for (int i = 0; i < 64; i++) {
    int num_pieces_here = 0;
    for (int j = 0; j < 12; j++) {
      if ((b->bitboard[j] >> i) & 1) num_pieces_here++;
    }
    if (num_pieces_here > 1) return 1;
  }

  // error 2: do the bitboard and piece board match?
  for (int i = 0; i < 64; i++) {
    if (b->piece_board[i] == NONE) continue;
    if (!((b->bitboard[b->piece_board[i]] >> i) & 1)) {
      return 2;
    }
  }

  // board passed all tests:
  return 0;
}

// todo: remove this
void check_material(board* b) {
  int total_material = 0;
  for (int i = 0; i < 64; i++) {
    total_material += PIECE_TO_MATERIAL[b->piece_board[i]];
  }

  if (total_material != b->material) {
    printf("%d != %d\n", total_material, b->material);
    b->print();
  }
}

int evaluate(board* b) {
  // naive evaluation for now:
  return b->material;
}

move alphabeta(board* b, int depth) {
  int best_score;
  int candidate_score;
  int alpha = -INF;
  int beta = INF;
  move best_move(NULL, NULL);
  int num_moves;

  // generate all possible moves from here:
  int* moves = b->get_moves(num_moves);

  // maximizing player is white:
  if (b->turn == WHITE) {
    best_score = -INF;

    // loop through all possible moves at this level and pick the best one:
    for (int i = 0; i < num_moves; i++) {
      // make the move:
      b->make_move(moves[i]);

      candidate_score = alphabeta_helper(b, depth-1, alpha, beta, false);
      if (candidate_score >= best_score) {
        best_score = candidate_score;
        best_move.move_code = moves[i];
        printf("better move (score %d) found: ", best_score);
        pm(moves[i]);
      }
      b->undo_move();

      // beta cutoff:
      alpha = std::max(alpha, best_score);
      if (alpha >= beta) break;
    }
  }

  // minimizing player is black:
  else {
    best_score = INF;

    // loop through all possible moves at this level and pick the best one:
    for (int i = 0; i < num_moves; i++) {
      // make the move:
      b->make_move(moves[i]);

      candidate_score = alphabeta_helper(b, depth-1, alpha, beta, true);
      if (candidate_score <= best_score) {
        best_score = candidate_score;
        best_move.move_code = moves[i];
      }

      b->undo_move();

      // alpha cutoff:
      if (beta <= alpha) break;
    }
  }

  // update and return the best move object:
  best_move.score = best_score;
  return best_move;
}

// alphabeta_helper(): returns the SCORE of the best move:
int alphabeta_helper(board* b, int depth, int alpha, int beta, bool maximizing) {
  // base case:
  if (depth == 0) return evaluate(b);

  check_material(b);

  // recursive case:
  int best_score;
  int num_moves;

  // generate all possible moves from this position:
  int* moves = b->get_moves(num_moves);

  // if we can't make any moves, it's either checkmate or stalemate:
  if (num_moves == 0) return b->is_check() ? -INF : 0;

  // if it's white's turn:
  if (maximizing) {
    best_score = -INF;

    // make every move and return the one with the highest value:
    for (int i = 0; i < num_moves; i++) {
      b->make_move(moves[i]);
      best_score = std::max(best_score, alphabeta_helper(b, depth-1, alpha, beta, false));
      b->undo_move();

      alpha = std::max(alpha, best_score);
      if (alpha >= beta) break;
    }
  }

  // if it's black's turn:
  else {
    best_score = INF;

    // make every move and return the one with the highest value:
    for (int i = 0; i < num_moves; i++) {
      b->make_move(moves[i]);
      best_score = std::min(best_score, alphabeta_helper(b, depth-1, alpha, beta, true));
      b->undo_move();

      beta = std::min(beta, best_score);
      if (beta <= alpha) break;
    }
  }

  delete[] moves;

  // return the best value found:
  return best_score;
}
