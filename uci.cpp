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
    else if (!strncmp(inbuf, "ucinewgame", 10)) parse_position("position startpos");
    else if (!strncmp(inbuf, "go", 2)) parse_go(inbuf);
    else if (!strncmp(inbuf, "quit", 4)) break;
    else if (!strncmp(inbuf, "print", 5)) b.print();
    else if (!strncmp(inbuf, "uci", 3)) {
      // print engine info:
      printf("id name %s\n", NAME);
      printf("id author %s\n", AUTHOR);
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
  // get depth:
  int depth = -1;
  char* depth_arg = strstr(command, "depth");
  if (depth_arg) depth = atoi(depth_arg + 6);
  else depth = 6;

  // now search the position with these parameters:
  search(depth);
}
