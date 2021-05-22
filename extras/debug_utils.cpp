// NOTE: this file is not meant to be compiled! it only contains code that
// is used for debugging!

/* divided_perft(): a divided perft routine.
 * given a board b and a depth, generates all legal moves to that depth and prints
 * the number of moves possible. functionally equivalent to typing 'go perft DEPTH'
 * into stockfish:
*/
int divided_perft(board* b, int depth) {
  int num_moves;
  int* moves = b->get_moves(num_moves);
  int total_moves = 0;
  for (int i = 0; i < num_moves; i++) {
    int to = MOVE_TO(moves[i]);
    int from = MOVE_FROM(moves[i]);

    // make the move:
    bool legal = b->make_move(moves[i]);
    int divided_moves = 0;
    if (legal) divided_moves = (depth > 1) ? perft(b, depth-1) : 1;
    b->undo_move();

    total_moves += divided_moves;
    if (legal) printf("%s%s: %d\n", square_name(from).c_str(), square_name(to).c_str(),
                               divided_moves);
  }
  return total_moves;
}

/* perft(): a helper function for the divided perft routine, or a standalone
 * perft function, minus the move division (i.e., it'll just output a single
 * number of moves):
*/
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

// print_bitboard(): a utility to visualize a single 64-bit bitboard:
void print_bitboard(U64 b) {
  for (int i = 0; i < 64; i++) {
    if ((b >> i) & 1) printf("x");
    else printf(".");

    if (i % 8 == 7) printf("\n");
  }
  printf("\n");
}

// interpret_move(): a utility to print out information about a given move int:
void interpret_move(int move) {
  int to = (move >> 26) & 0x3F;
  int from = (move >> 20) & 0x3F;
  int captured = (move >> 16) & 0xF;
  int ep = (move >> 15) & 1;
  int pf = (move >> 14) & 1;
  int castle = (move >> 13) & 1;
  int promotion = (move >> 12) & 1;
  int promoted_piece = (move >> 8) & 0xF;
  int pcr = (move >> 4) & 0xF;
  int piece_moved = move & 0xF;

  printf("%c moved from %s to %s\n", PIECE_CHARS[piece_moved], square_name(from).c_str(), square_name(to).c_str());
  if (captured != NONE) printf("\tcapture: %c\n", PIECE_CHARS[captured]);
  if (ep) printf("\tEN PASSANT\n");
  if (pf) printf("\tPAWN'S FIRST MOVE\n");
  if (castle) printf("\tCASTLE\n");
  if (promotion) printf("\tPROMOTION TO %c\n", PIECE_CHARS[promoted_piece]);
}

// square_name(): converts a bitboard index (0-63) to its corresponding square name:
std::string square_name(char i) {
  std::string s = "";
  s += (char) ((i%8) + 'a');
  s += (char) (8 - (i/8) + '0');
  return s;
}

// diff(): are boards a and b the same? used for debugging undo_move()
bool diff(board* a, board* b) {
  if (memcmp(&(a->bitboard), &(b->bitboard), 12 * sizeof(U64))) return true;
  if (memcmp(&(a->piece_board), &(b->piece_board), 64 * sizeof(char))) return true;
  if (a->num_moves_played != b->num_moves_played) return true;
  if (memcmp(&(a->move_history), &(b->move_history), a->num_moves_played)) return true;
  if (a->material != b->material) return true;
  if (a->turn != b->turn) return true;
  if (a->castle_rights != b->castle_rights) return true;

  return false;
}
