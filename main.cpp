#include <assert.h>
#include <stdio.h>
#include <string>

#include "board.h"
#include "consts.h"
#include "defs.h"
#include "engine.h"
#include "timer.h"

void print_move(int m);

int main(int argc, char** argv) {
  init_consts();
  init_search();

  char* FEN = "r1bqkbnr/ppp2ppp/2n5/3pp3/3PP3/5N2/PPP2PPP/RNBQKB1R w KQkq d6 0 4";
  // board b(FEN);

  /* timer t;
  t.start();
  search s(&b);
  print_move(s.negamax(4));
  printf("evaluated %d nodes\n", s.nodes_evaluated);

  printf("PV: ");
  for (int i = 0; i < s.pv_length[0]; i++) {
    print_move(s.pv_table[0][i]);
  }
  printf("\n");

  t.stop();
  t.print_duration(); */

  b = board("r1bq2r1/b4pk1/p1pp1p2/1p2pP2/1P2P1PB/3P4/1PPQ2P1/R3K2R w");
  b.print();
  search(5);

  // print the principal variation:
  for (int i = 0; i < pv_length[0]; i++) {
    print_move(pv_table[0][i]);
  }
  printf("\n");

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

void print_move(int m) {
  int to = MOVE_TO(m);
  int from = MOVE_FROM(m);

  char* move_str = new char[6];
  move_str[5] = '\0';
  move_str[4] = '\0'; // TODO: REMOVE THIS

  move_str[0] = ((from % 8) + 'a');
  move_str[1] = (8 - (from / 8) + '0');
  move_str[2] = ((to % 8) + 'a');
  move_str[3] = (8 - (to / 8) + '0');

  // TODO: promotion piece
  std::string moveinfo = (MOVE_IS_EP(m)) ? " EP" : "";
  printf("%s%s ", move_str, moveinfo.c_str());
}
