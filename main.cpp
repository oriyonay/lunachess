#include <assert.h>
#include <stdio.h>

#include "board.h"
#include "consts.h"
#include "defs.h"
#include "timer.h"

int perft(board* b, int depth);

int main(int argc, char** argv) {
  init_consts();

  char* FEN = FEN_START;
  board b(FEN);

  timer t;
  t.start();
  printf("%d total moves\n", perft(&b, 5));
  t.stop();
  t.print_duration();
  // b.print();

  return 0;
}

int perft(board* b, int depth) {
  int num_moves;
  int* moves = b->get_moves(num_moves);
  if (depth == 1) {
    int n_copy = num_moves;
    for (int i = 0; i < n_copy; i++) {
      if (!b->make_move(moves[i])) num_moves--;
      b->undo_move();
    }
    return num_moves;
  }

  int sum = 0;
  for (int i = 0; i < num_moves; i++) {
    if (b->make_move(moves[i])) {
      sum += perft(b, depth - 1);
    }
    b->undo_move();
  }

  return sum;
}
