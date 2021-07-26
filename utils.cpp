#include "utils.h"

// print a move given the move code. does NOT print a new line!
void print_move(int move) {
  int to = MOVE_TO(move);
  int from = MOVE_FROM(move);

  char* move_str = new char[6];

  move_str[0] = ((from % 8) + 'a');
  move_str[1] = (8 - (from / 8) + '0');
  move_str[2] = ((to % 8) + 'a');
  move_str[3] = (8 - (to / 8) + '0');
  move_str[4] = (MOVE_IS_PROMOTION(move)) ?
                  PIECE_CHARS[MOVE_PROMOTION_PIECE(move)] : '\0';
  move_str[5] = '\0';

  printf("%s", move_str);
}

// gets the time in milliseconds (as an int - we assume no one's
// going to run this for 50 days straight)
int get_time() {
  #ifdef WIN32
    return GetTickCount();
  #else
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec * 1000) + (t.tv_usec / 1000);
  #endif
}
