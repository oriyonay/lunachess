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

  char* FEN = "r1b1kbnr/pppp1Npp/8/8/3nq3/8/PPPPBP1P/RNBQKR2 b Qkq - 1 7";
  board b(FEN);

  timer t;
  t.start();
  search s(&b);
  print_move(s.negamax(5));
  printf("evaluated %d nodes\n", s.nodes_evaluated);
  t.stop();
  t.print_duration();

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
  printf("%s%s\n", move_str, moveinfo.c_str());
}
