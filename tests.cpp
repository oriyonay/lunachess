#include <stdio.h>
#include <string>
#include <vector>

#include "board.h"
#include "timer.h"

#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

long perft(board* b, int depth);
long perft_verify(board* b, int depth);
bool verify(board* b);
void print_move(int m);

struct perft_test {
  char* test_name;
  char* FEN;
  std::vector<long> results;

  perft_test(char* test_name, char* FEN, std::vector<long> results) :
    test_name(test_name), FEN(FEN), results(results) {}

  bool test() {
    board b(FEN);
    printf("starting test: %s\n", test_name);
    verify(&b);
    for (int i = 1; i < results.size(); i++) {
      if (perft(&b, i) != results[i]) {
        printf("%s %sFAILED%s at depth %d\n", test_name, RED, RESET, i);
        return false;
      }
    }
    printf("%s %sPASSED%s.\n", test_name, GREEN, RESET);
    return true;
  }
};

int main() {
  // initialize engine constants!
  init_consts();

  // create test objects:
  perft_test initial_position(
    "initial position",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ",
    std::vector<long> {1, 20, 400, 8902, 197281, 4865609, 119060324}
  );

  perft_test pt2 (
    "perft test 2",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
    std::vector<long> {1, 48, 2039, 97862, 4085603, 193690690}
  );

  perft_test pt3 (
    "perft test 3",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ",
    std::vector<long> {1, 14, 191, 2812, 43238, 674624, 11030083, 178633661}
  );

  perft_test pt4 (
    "perft test 4",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    std::vector<long> {1, 6, 264, 9467, 422333, 15833292}
  );

  perft_test pt5 (
    "perft test 5",
    "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ",
    std::vector<long> {1, 6, 264, 9467, 422333, 15833292}
  );

  perft_test pt6 (
    "perft test 6",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ",
    std::vector<long> {1, 44, 1486, 62379, 2103487, 89941194}
  );

  perft_test pt7 (
    "perft test 7",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ",
    std::vector<long> {1, 46, 2079, 89890, 3894594, 164075551}
  );

  // run all tests:
  timer t;
  t.start();
  bool all_tests_passed = true;
  all_tests_passed &= initial_position.test();
  all_tests_passed &= pt2.test();
  all_tests_passed &= pt3.test();
  all_tests_passed &= pt4.test();
  all_tests_passed &= pt5.test();
  all_tests_passed &= pt6.test();
  all_tests_passed &= pt7.test();
  t.stop();

  if (all_tests_passed) printf("%sALL TESTS PASSED%s.\n", GREEN, RESET);
  else printf("%sNOT ALL TESTS PASSED%s.\n", YELLOW, RESET);
  t.print_duration();
}

// perft(): the perft test function
long perft(board* b, int depth) {
  int num_moves;
  int* moves = b->get_moves(num_moves);
  if (depth == 1) {
    delete[] moves;
    return num_moves;
  }

  long sum = 0;
  for (int i = 0; i < num_moves; i++) {
    b->make_move(moves[i]);
    sum += perft(b, depth - 1);
    b->undo_move();
  }

  delete[] moves;

  return sum;
}

// perft_verify(): perft testing using the verify() method
long perft_verify(board* b, int depth) {
  if (depth == 0) {
    verify(b);
    return 1;
  }
  int num_moves;
  int* moves = b->get_moves(num_moves);

  long sum = 0;
  for (int i = 0; i < num_moves; i++) {
    b->make_move(moves[i]);
    sum += perft_verify(b, depth - 1);
    b->undo_move();
  }

  delete[] moves;
  return sum;
}

// verify(): verifies the validity of the state of the board
bool verify(board* b) {
  // make sure the bitboard and the piece board (mailbox piece list) are synced:
  for (int sq = 0; sq < 64; sq++) {
    if (b->piece_board[sq] != NONE) {
      if (!((b->bitboard[b->piece_board[sq]] >> (sq)) & 1)) {
        printf("BOARD AND MAILBOX LIST ARE INCONSISTENT\n");
        printf("bitboard does not contain bit for piece_board[%d]\n", sq);
        assert(false);
      }
    }
    // otherwise, verify that no bitboard contains a 1 here:
    else {
      for (int piece = 0; piece < 12; piece++) {
        if ((b->bitboard[piece] >> sq) & 1) {
          printf("BOARD AND MAILBOX LIST ARE INCONSISTENT\n");
          printf("bitboard[%d] contains set bit for empty piece_board entry [%d]", piece, sq);
          assert(false);
        }
      }
    }
  }

  // make sure the board's base_score equals its actual base score:
  int total = 0;
  for (int i = 0; i < 64; i++) {
    if (b->piece_board[i] == NONE) continue;
    total += PIECE_SQUARE_TABLE[b->piece_board[i]][i];
  }

  if (total != b->base_score) {
    printf("PST SCORES DIFFER\n", total, b->base_score);
    assert(false);
  }

  // make sure zobrist keys are correctly calculated:
  U64 zobrist = ZOBRIST_CASTLE_RIGHTS_KEYS[b->castle_rights];
  if (b->turn == WHITE) zobrist ^= ZOBRIST_TURN_KEY;
  for (int sq = 0; sq < 64; sq++) {
    if (b->piece_board[sq] == NONE) continue;
    zobrist ^= ZOBRIST_SQUARE_KEYS[b->piece_board[sq]][sq];
  }

  if (zobrist != b->hash) {
    printf("INCORRECT ZOBRIST HASH\n");
    assert(false);
  }
}

// print_move(): prints a given a move int. placed here temporarily, since there
// isn't a utils file. this exact code is also in main.cpp for now
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
  std::string epinfo = MOVE_IS_EP(m) ? " EP" : "";
  std::string capinfo = (MOVE_CAPTURED(m) != NONE) ? " CAPTURE" : "";
  printf("%s%s%s\n", move_str, epinfo.c_str(), capinfo.c_str());
}
