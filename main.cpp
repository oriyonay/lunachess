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
  b = board(FEN_START);
  uci_loop();

  return 0;
}
