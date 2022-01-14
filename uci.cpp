#include "uci.h"

// uci_loop(): start the UCI communication protocol
void uci_loop() {
  // reset stdin and stdout buffers:
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  // create an input buffer:
  char inbuf[DEFAULT_UCI_INPUT_BUFFER_SIZE];

  // main UCI loop:
  while (true) {
    // clear input buffer:
    memset(inbuf, 0, DEFAULT_UCI_INPUT_BUFFER_SIZE);

    // flush output stream:
    fflush(stdout);

    // make sure we got the user input:
    if (!fgets(inbuf, DEFAULT_UCI_INPUT_BUFFER_SIZE, stdin)) continue;
    if (inbuf[0] == '\n') continue;

    // now we have the user input. start parsing commands:
    if (!strncmp(inbuf, "isready", 7)) printf("readyok\n");
    else if (!strncmp(inbuf, "position", 8)) parse_position(inbuf);
    else if (!strncmp(inbuf, "ucinewgame", 10)) {
      // clear the transposition table and reset the board:
      TT.clear();
      parse_position("position startpos");
    }
    else if (!strncmp(inbuf, "go", 2)) parse_go(inbuf);
    else if (!strncmp(inbuf, "quit", 4)) break;
    else if (!strncmp(inbuf, "print", 5)) b.print();
    else if (!strncmp(inbuf, "setoption", 9)) parse_option(inbuf);
    else if (!strncmp(inbuf, "uci", 3)) {
      // print engine info:
      printf("id name %s\n", NAME);
      printf("id author %s\n", AUTHOR);
      printf("option name SyzygyPath type string default None\n");
      printf("uciok\n");
    }
  }
}

// parse 'position' command:
void parse_position(char* command) {
  // move command pointer to the point immediately after 'position ':
  command += 9;

  // handle 'position startpos':
  if (!strncmp(command, "startpos", 8)) {
    b = board(FEN_START);
  }
  // if it's not 'startpos' then we're parsing a FEN string:
  else {
    // make sure 'fen' is the next part of our command. if not, just reset the board:
    if (strncmp(command, "fen", 3)) {
      b = board(FEN_START);
    }
    // otherwise, we can parse the FEN as expected:
    else {
      // move pointer until after 'fen '
      command += 4;

      // load FEN:
      b = board(command);
    }
  }

  // now we handle any 'moves'. jump to 'moves' location, or return if this
  // is the end of the string:
  command = strstr(command, "moves");
  if (!command) return;

  // jump to position immediately after 'moves':
  command += 5;

  // make all moves:
  while (true) {
    // move command pointer to next move (occurrence of ' ') and make sure it's
    // not the end of the string:
    command = strstr(command, " ");
    if (!command) break;

    // move command pointer to the point after the space:
    command++;

    // make the move on the board:
    b.make_move(command);
  }
}

// parse 'go' command:
void parse_go(char* command) {
  // initialize variables:
  time_set = false;
  moves_to_go = -1;
  moves_to_go_enabled = false;
  move_time = -1;
  time_limit = -1;
  time_increment = 0;
  start_time = get_time();

  // parse depth command:
  int depth = -1;
  char* arg = strstr(command, "depth");
  if (arg) depth = atoi(arg + 6);

  // parse perft command:
  arg = strstr(command, "perft");
  if (arg) {
    depth = atoi(arg + 6);
    printf("\nnodes searched: %llu\n", perft(depth, true));
    return;
  }

  arg = strstr(command, "binc");
  if (arg && (b.turn == BLACK)) time_increment = atoi(arg + 5);

  arg = strstr(command, "winc");
  if (arg && (b.turn == WHITE)) time_increment = atoi(arg + 5);

  arg = strstr(command, "btime");
  if (arg && (b.turn == BLACK)) time_limit = atoi(arg + 6);

  arg = strstr(command, "wtime");
  if (arg && (b.turn == WHITE)) time_limit = atoi(arg + 6);

  arg = strstr(command, "movestogo");
  if (arg) {
    moves_to_go_enabled = true;
    moves_to_go = atoi(arg + 10);
  }

  arg = strstr(command, "movetime");
  if (arg) move_time = atoi(arg + 9);

  if (move_time != -1) {
    time_limit = move_time;
    moves_to_go = 1;
  }

  else if (time_limit != -1) {
    time_set = true;

    int k;
    if (moves_to_go_enabled) {
      k = moves_to_go;
    }
    else {
      int num_pieces = __builtin_popcountll(b.W | b.B);
      k = 40 - (32 - num_pieces);
    }

    time_limit = (time_limit / k) + (time_increment / 2);
    time_limit = std::max(time_limit, 50);

    stop_time = start_time + time_limit - 30;
  }

  // if depth is not set, search 'infinitely':
  if (depth == -1) depth = 64;

  // now search the position with these parameters:
  search(depth);
}

void parse_option(char* command) {
  command += 10;

  // command must start with 'setoption name'
  if (strncmp(command, "name", 4)) {
    printf("error: option not recognized.\n");
    return;
  }

  command += 5;

  if (!strncmp(command, "SyzygyPath", 10)) {
    // expect next characters to be 'SyzygyPath value '
    command += 17;

    // we have to get rid of the newline in the input buffer before feeding
    // it to fathom!
    char* endc = strchr(command, '\n');
    if (endc) *endc = 0;

    // remove trailing whitespace:
    char* end = command + strlen(command) - 1;
    while(end > command && isspace((unsigned char)* end)) end--;
    end[1] = 0;

    // initialize the tablebase:
    tb_init(command);

    // output info to the user:
    if (TB_LARGEST > 0) {
      printf("info string largest tablebase size = %d\n", TB_LARGEST);
    }
    else {
      printf("info string error: tablebase not found\n");
    }
  }
}
