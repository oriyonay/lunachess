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

      printf("ROOT MOVE MADE\n");

      candidate_score = alphabeta_helper(b, depth-1, alpha, beta, false);
      if (candidate_score >= best_score) {
        best_score = candidate_score;
        best_move.move_code = moves[i];
      }
      alpha = std::max(alpha, best_score);

      // beta cutoff:
      if (alpha >= beta) break;
      b->undo_move();
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

      // alpha cutoff:
      if (beta <= alpha) break;
      b->undo_move();
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
    printf("starting maximizing loop\n");
    for (int i = 0; i < num_moves; i++) {
      b->make_move(moves[i]);
      pm(moves[i]);
      printf("move: %d, depth: %d\n", moves[i], depth);
      b->print();
      best_score = std::max(best_score, alphabeta_helper(b, depth-1, alpha, beta, false));
      alpha = std::max(alpha, best_score);
      if (alpha >= beta) break;
      b->undo_move();
    }
  }

  // if it's black's turn:
  else {
    best_score = INF;

    // make every move and return the one with the highest value:
    printf("starting minimizing loop\n");
    for (int i = 0; i < num_moves; i++) {
      b->make_move(moves[i]);
      pm(moves[i]);
      printf("move: %d, depth: %d\n", moves[i], depth);
      b->print();
      best_score = std::min(best_score, alphabeta_helper(b, depth-1, alpha, beta, true));
      beta = std::min(beta, best_score);
      if (beta <= alpha) break;
      b->undo_move();
    }
  }

  delete[] moves;

  // return the best value found:
  return best_score;
}
