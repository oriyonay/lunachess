#include <assert.h>
#include <stdio.h>
#include <string>

#include "board.h"
#include "consts.h"
#include "defs.h"
#include "engine.h"
#include "uci.h"
#include "utils.h"

int main(int argc, char** argv) {
  init_consts();
  start_uci();

  /* b = board("rnbq4/pppp3Q/3bk1p1/8/4P3/8/PPPP1PPP/RNB1KB1R w KQ - 1 9");
  b.print();
  search(6);

  // print the principal variation:
  for (int i = 0; i < pv_length[0]; i++) {
    print_move(pv_table[0][i]);
    printf(" ");
  }
  printf("\n");
  printf("nodes evaluated: %d\n", nodes_evaluated); */

  /* timer t;
  int move;

  while (true) {
    // human makes move:
    char human_move[6];
    scanf("%s", human_move);
    b.make_move(human_move);
    b.print();

    // engine makes move:
    t.start();
    move = negamax(5);
    print_move(move);
    printf("evaluated %d nodes\n", nodes_evaluated);
    b.make_move(move);
    b.print();

    t.stop();
    t.print_duration();
  } */

  return 0;
}
