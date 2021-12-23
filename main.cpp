#include <assert.h>
#include <stdio.h>
#include <string>

#include "board.h"
#include "consts.h"
#include "defs.h"
#include "engine.h"
#include "eval_params.h"
#include "tt.h"
#include "uci.h"
#include "utils.h"

int main(int argc, char** argv) {
  init_consts();
  init_eval_params();
  init_globals();
  uci_loop();

  return 0;
}
