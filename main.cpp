#include <assert.h>
#include <stdio.h>
#include <string>

#include "board.h"
#include "consts.h"
#include "defs.h"
#include "engine.h"
#include "tt.h"
#include "uci.h"
#include "utils.h"

int main(int argc, char** argv) {
  init_consts();
  init_globals();
  update_tt(10, 25, TT_EXACT);
  // printf("%d\n", probe_tt(4, 19, 30));
  uci_loop();

  return 0;
}
